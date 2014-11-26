#include "stdio.h"
#include "stdlib.h"
#include "malloc.h"
#include "libvvplot_api.h"

#include <vector>
// #include <deque>
#include <list>
#include <limits>

//#include "core.h"
using namespace std;

static const float NaN = numeric_limits<float>::quiet_NaN();

class vec_t
{
	public:
		float x, y;
		vec_t(): x(0),y(0) {}
		vec_t(float _x, float _y): x(_x), y(_y) {}
		inline bool operator==(const vec_t &v) const { return v.x == x && v.y == y; }
};

/*class seg_t
{
	public:
		vec_t v1, v2;
		float c;
		seg_t(): v1(),v2(),c() {}
		seg_t(vec_t _v1, vec_t _v2, float _c): v1(_v1), v2(_v2), c(_c) {}
};*/

static float max(float a1, float a2) {return (a1>a2)?a1:a2;}
static float min(float a1, float a2) {return (a1<a2)?a1:a2;}
static bool inrange(float z1, float z2, float c) { return (z1 <= c && c < z2) || (z1 >= c && c > z2); }

typedef vector<vec_t> line_t;
static list<line_t*> *isolines = NULL;

void merge_lines(line_t* dst, bool dst_side)
{
	vec_t vec = dst_side ? dst->front() : dst->back();

	for (auto lit = isolines->begin(); lit != isolines->end(); lit++)
	{
		line_t *l = *lit;
		if (l == dst) continue;
		else if (l->front() == vec)
		{
			if (dst_side) dst->insert(dst->begin(), l->rbegin(), l->rend());
			else          dst->insert(dst->end(),   l->begin(), l->end());
		}
		else if (l->back() == vec)
		{
			if (dst_side) dst->insert(dst->begin(), l->begin(), l->end());
			else          dst->insert(dst->end(),   l->rbegin(), l->rend());
		}
		else continue;

		delete *lit;
		isolines->erase(lit);
		break;
	}
}

void commit_segment(vec_t vec1, vec_t vec2)
{
	for (auto it = isolines->begin(); it!= isolines->end(); it++)
	{
		line_t *l = *it;

		     if (l->front() == vec1) { l->insert(l->begin(), vec2); merge_lines(l, 1); return; }
		else if (l->front() == vec2) { l->insert(l->begin(), vec1); merge_lines(l, 1); return; }
		else if (l->back()  == vec1) { l->insert(l->end(),   vec2); merge_lines(l, 0); return; }
		else if (l->back()  == vec2) { l->insert(l->end(),   vec1); merge_lines(l, 0); return; }
	}
	line_t *new_line = new line_t();
	new_line->push_back(vec1);
	new_line->push_back(vec2);
	isolines->push_back(new_line);
	return;
}

void process_rect(float x, float y, float z[4], float C)
{
	vec_t vecs[4];
	int N = 0;
	if (inrange(z[0], z[1], C)) vecs[N++] = vec_t(x,                        y + (C-z[0])/(z[1]-z[0]));
	if (inrange(z[1], z[2], C)) vecs[N++] = vec_t(x + (C-z[1])/(z[2]-z[1]), y + 1.0);
	if (inrange(z[2], z[3], C)) vecs[N++] = vec_t(x + 1.0,                  y + (C-z[3])/(z[2]-z[3]));
	if (inrange(z[3], z[0], C)) vecs[N++] = vec_t(x + (C-z[0])/(z[3]-z[0]), y);
	
	if (N>=2) commit_segment(vecs[0], vecs[1]);
	if (N>=4) commit_segment(vecs[2], vecs[3]);
}

int map_isoline(hid_t fid, const char *dsetname, float *cvals, int cnum)
{
	if (!isolines) isolines = new list<line_t*>();

	H5Eset_auto2(H5E_DEFAULT, NULL, NULL);
	hid_t dataset = H5Dopen2(fid, dsetname, H5P_DEFAULT);
	if (dataset < 0)
	{
		H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
		H5Eprint2(H5E_DEFAULT, stderr);
		fprintf(stderr, "error: argument dataset: can't open dataset '%s'\n", dsetname);
		return 3;
	}

	double xmin, xmax, ymin, ymax, spacing;
	attribute_read_double(dataset, "xmin", xmin);
	attribute_read_double(dataset, "xmax", xmax);
	attribute_read_double(dataset, "ymin", ymin);
	attribute_read_double(dataset, "ymax", ymax);
	attribute_read_double(dataset, "spacing", spacing);

	hid_t dataspace = H5Dget_space(dataset);
	if (dataspace < 0)
	{
		H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
		H5Eprint2(H5E_DEFAULT, stderr);
		return 5;
	}
	hsize_t dims[2];
	H5Sget_simple_extent_dims(dataspace, dims, dims);
	float *mem = (float*)malloc(sizeof(float)*dims[0]*dims[1]);
	herr_t err = H5Dread(dataset, H5T_NATIVE_FLOAT, H5S_ALL, dataspace, H5P_DEFAULT, mem);
	if (err < 0)
	{
		H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
		H5Eprint2(H5E_DEFAULT, stderr);
		return 5;
	}
	
	// fprintf(stdout, "%lf %lf %lf %lf %lf\n", xmin, xmax, ymin, ymax, spacing);
	// fwrite(args, sizeof(double), 5, stdout);
	for (int i=0; i<dims[0]-1; i++)
	{
		float x = xmin + i*spacing;
		for (int j=0; j<dims[1]-1; j++)
		{
			float y = ymin + j*spacing;

			// 1 2
			// 0 3
			float corners[4] = {mem[(i+0)*dims[1]+(j+0)], mem[(i+0)*dims[1]+(j+1)],
			                    mem[(i+1)*dims[1]+(j+1)], mem[(i+1)*dims[1]+(j+0)]};
			for (int c=0; c<cnum; c++)
				process_rect(i, j, corners, cvals[c]);
		}
	}

	// if (vec_t(17.333, 15.00004) == vec_t(17.333, 15.00004))
	// 	{printf("ok\n");}
	// else
	// 	{printf("boo\n");}

	for (auto it = isolines->begin(); it!= isolines->end(); it++)
	{
		line_t *l = *it;
		for (auto vec = l->begin(); vec!=l->end(); vec++)
		{
			float xy[2] = { xmin+vec->x*spacing, ymin+vec->y*spacing };
			// printf("%g %g\n", xy[0], xy[1]);
			fwrite(xy, sizeof(float), 2, stdout);
		}
		float nans[2] = {NaN, NaN};
		fwrite(nans, sizeof(float), 2, stdout);
		// printf("\n");
	}
	fflush(stdout);
	// assert(c == dims[0]*dims[1]);
	// fwrite(dims, sizeof(hsize_t), 2, stdout);
	// fwrite(&xmin, sizeof(double), 1, stdout);

	delete isolines;
	free(mem);
	H5Sclose(dataspace);
	H5Dclose(dataset);
	// printf("%s\n", argv[0]);
	return 0;
}

// int main(int argc, char **argv)
// {
// 	hid_t fid = H5Fopen(argv[1], H5F_ACC_RDONLY, H5P_DEFAULT);

// 	float c[2] = {0, 1};
// 	map_isoline(fid, argv[2], c, 2);
// 	return 0;
// }