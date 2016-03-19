#include "stdio.h"
#include "stdlib.h"
#include "malloc.h"
#include "libvvplot_api.h"

#include <vector>
// #include <deque>
#include <list>
#include <limits>

static const float NaN = std::numeric_limits<float>::quiet_NaN();
static bool inrange(float z1, float z2, float c) { return (z1 <= c && c < z2) || (z1 >= c && c > z2); }

class vec_t
{
	public:
		float x, y;
		vec_t(): x(0),y(0) {}
		vec_t(float _x, float _y): x(_x), y(_y) {}
		inline bool operator==(const vec_t &v) const { return v.x == x && v.y == y; }
};

typedef struct {
	uint16_t xi;
	uint16_t yj;
	float psi_gap;
} gap_t;

typedef std::vector<vec_t> line_t;
static std::list<line_t*> *isolines = NULL;

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

void process_rect(float x, float y, float z[5], float C)
{
	vec_t vecs[4];
	int N = 0;
	if (inrange(z[0], z[1], C)) vecs[N++] = vec_t(x,                        y + (C-z[0])/(z[1]-z[0]));
	if (inrange(z[1], z[2], C)) vecs[N++] = vec_t(x + (C-z[1])/(z[2]-z[1]), y + 1.0);
	if (inrange(z[2], z[3], C)) vecs[N++] = vec_t(x + 1.0,                  y + (C-z[3])/(z[2]-z[3]));
	if (inrange(z[3], z[4], C)) vecs[N++] = vec_t(x + (C-z[4])/(z[3]-z[4]), y);

	if (N>=2) commit_segment(vecs[0], vecs[1]);
	if (N>=4) commit_segment(vecs[2], vecs[3]);
}

int map_isoline(hid_t fid, const char *dsetname, float *cvals, int cnum)
{
	if (!isolines) isolines = new std::list<line_t*>();
	herr_t err = 0;

	H5Eset_auto2(H5E_DEFAULT, NULL, NULL);
	hid_t map_h5d = H5Dopen2(fid, dsetname, H5P_DEFAULT);
	if (map_h5d < 0)
	{
		H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
		H5Eprint2(H5E_DEFAULT, stderr);
		fprintf(stderr, "error: argument dataset: can't open dataset '%s'\n", dsetname);
		return 3;
	}
	hid_t map_h5s = H5Dget_space(map_h5d);
	if (map_h5s < 0)
	{
		H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
		H5Eprint2(H5E_DEFAULT, stderr);
		return 5;
	}

	hsize_t dims[2];
	H5Sget_simple_extent_dims(map_h5s, dims, dims);
	float *mem = (float*)malloc(sizeof(float)*dims[0]*dims[1]);
	err = H5Dread(map_h5d, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, mem);
	if (err < 0)
	{
		H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
		H5Eprint2(H5E_DEFAULT, stderr);
		return 5;
	}

	double xmin, xmax, ymin, ymax, spacing;
	attribute_read_double(map_h5d, "xmin", xmin);
	attribute_read_double(map_h5d, "xmax", xmax);
	attribute_read_double(map_h5d, "ymin", ymin);
	attribute_read_double(map_h5d, "ymax", ymax);
	attribute_read_double(map_h5d, "spacing", spacing);

	std::vector<gap_t> gap_list;
	if (H5Aexists(map_h5d, "gaps"))
	{
		hid_t gap_h5t = H5Tcreate(H5T_COMPOUND, 8);
		H5Tinsert(gap_h5t, "xi", 0, H5T_NATIVE_UINT16);
		H5Tinsert(gap_h5t, "yj", 2, H5T_NATIVE_UINT16);
		H5Tinsert(gap_h5t, "gap", 4, H5T_NATIVE_FLOAT);

		hid_t gap_h5a = H5Aopen(map_h5d, "gaps", H5P_DEFAULT);
		hid_t gap_h5s = H5Aget_space(gap_h5a);
		hsize_t gap_dims;
		H5Sget_simple_extent_dims(gap_h5s, &gap_dims, &gap_dims);

		gap_list.resize(gap_dims);
		err = H5Aread(gap_h5a, gap_h5t, gap_list.data());
		if (err < 0)
		{
			H5Epop(H5E_DEFAULT, H5Eget_num(H5E_DEFAULT)-1);
			H5Eprint2(H5E_DEFAULT, stderr);
			return 5;
		}

		H5Sclose(gap_h5s);
		H5Aclose(gap_h5a);
		H5Tclose(gap_h5t);
	}

	for (size_t i=0; i<dims[0]-1; i++)
	{
		// float x = xmin + i*spacing;
		for (size_t j=0; j<dims[1]-1; j++)
		{
			// float y = ymin + j*spacing;

			// 1 2
			// 0 3
			float corners[5] = {mem[(i+0)*dims[1]+(j+0)], mem[(i+0)*dims[1]+(j+1)],
			                    mem[(i+1)*dims[1]+(j+1)], mem[(i+1)*dims[1]+(j+0)],
			                    mem[(i+0)*dims[1]+(j+0)]};

			for (gap_t& gap: gap_list)
			{
				if (gap.yj != j)
					continue;
				if (gap.xi >= i)
					corners[0] += gap.psi_gap;
				if (gap.xi > i) {
					corners[3] += gap.psi_gap;
					corners[4] += gap.psi_gap;
				}

			}

			for (int c=0; c<cnum; c++)
				process_rect(i, j, corners, cvals[c]);
		}
	}

	for (auto it = isolines->begin(); it!= isolines->end(); it++)
	{
		line_t *l = *it;
		for (auto vec = l->begin(); vec!=l->end(); vec++)
		{
			float xy[2] = { static_cast<float>(xmin+vec->x*spacing), static_cast<float>(ymin+vec->y*spacing) };
			fwrite(xy, sizeof(float), 2, stdout);
		}
		float nans[2] = {NaN, NaN};
		fwrite(nans, sizeof(float), 2, stdout);
	}
	fflush(stdout);

	delete isolines;
	free(mem);
	H5Sclose(map_h5s);
	H5Dclose(map_h5d);
	return 0;
}
