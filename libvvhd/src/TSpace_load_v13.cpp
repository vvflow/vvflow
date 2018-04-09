#include "TSpace.hpp"

static int eq(const char *str1, const char *str2)
{
    for (int i=0; i<8; i++)
    {
        if (str1[i] != str2[i]) return i+1;
    }
    return 9;
}

static void LoadList(std::vector<TObj> &list, FILE* fin)
{
    int64_t size;
    (void)fread(&size, 8, 1, fin);
    TObj obj;
    for (int64_t i=0; i<size; i++)
    {
        fread(&obj, 24, 1, fin);
        list.push_back(obj);
    }
}

static void read_shell_script(FILE* file, TEval &script)
{
    int32_t len;
    fread(&len, 4, 1, file);
    char str[len+1];
    fread(&str, 1, len, file);
    str[len] = 0;
    script = str;
}

void Space::load_v13(const char* fname)
{
    FILE *fin = fopen(fname, "rb");
    if (!fin) { perror("Error loading the space"); return; }

    //loading different lists
    int64_t tmp = 0;
    char comment[9] = {};
    for (int i=0; i<64; i++)
    {
        fseek(fin, i*16, SEEK_SET);
        fread(&tmp, 8, 1, fin);
        fread(comment, 8, 1, fin);
        if (!tmp) continue;
        fseek(fin, tmp, SEEK_SET);

        if (eq(comment, "Header  ")>8)
        {
            char version[8]; fread(&version, 8, 1, fin);
            if (eq(version, "v: 1.3  ") <= 8)
            {
                fprintf(stderr, "Cant read binary file version \"%s\".\n", version);
                exit(1);
            }

            char name[64];
            fread(name, 1, 64, fin);
            caption = name;
            fread(&time, 8, 1, fin);
            fread(&dt, 8, 1, fin);
            fread(&dt_save, 8, 1, fin);
            fread(&dt_streak, 8, 1, fin);
            fread(&dt_profile, 8, 1, fin);
            fread(&re, 8, 1, fin);
            fread(&pr, 8, 1, fin);
            fread(&inf_marker, 16, 1, fin);
            read_shell_script(fin, inf_vx);
            read_shell_script(fin, inf_vy);
            fread(&inf_g, 8, 1, fin);
            fread(&gravity, 16, 1, fin);
            fread(&finish, 8, 1, fin);

            int64_t rawtime; fread(&rawtime, 8, 1, fin);
        }
        else if (eq(comment, "Vortexes")>8) LoadList(VortexList, fin);
        else if (eq(comment, "Heat    ")>8) LoadList(HeatList, fin);
        else if (eq(comment, "StrkSrc ")>8) LoadList(StreakSourceList, fin);
        else if (eq(comment, "Streak  ")>8) LoadList(StreakList, fin);
        else if (eq(comment, "BData   ")>8)
        {
            std::shared_ptr<TBody> body(new TBody());

            fread(&body->holder, 24, 1, fin);
            fread(&body->dpos, 24, 1, fin);
            read_shell_script(fin, body->speed_x);
            read_shell_script(fin, body->speed_y);
            read_shell_script(fin, body->speed_o);
            fread(&body->speed_slae, 24, 1, fin);
            fread(&body->speed_slae_prev, 24, 1, fin);

            fread(&body->kspring, 24, 1, fin);
            fread(&body->density, 8, 1, fin);

            fread(&body->force_hydro, 24, 1, fin);
            fread(&body->fdt_dead, 24, 1, fin);
            fread(&body->friction_prev, 24, 1, fin);

            BodyList.push_back(body);
        }
        else if (eq(comment, "Body    ")>8)
        {
            auto body = BodyList.back();

            TAtt att(0.0, 0.0);
            int64_t size; fread(&size, 8, 1, fin);
            for (int64_t i=0; i<size; i++)
            {
                int64_t bc, hc;
                double heat_const;
                fread(&att.corner, 16, 1, fin);
                fread(&att.g, 8, 1, fin);
                fread(&bc, 8, 1, fin);
                fread(&hc, 8, 1, fin);
                fread(&heat_const, 8, 1, fin);
                fread(&att.gsum, 8, 1, fin);
                att.heat_const = heat_const;
                if (bc == 'l' || bc == 'n')
                    att.slip = bc == 'l';
                else
                    switch (bc)
                    {
                        case 's': 
                        case 'i': body->boundary_condition = bc_t::steady; break;
                        case 'z': body->boundary_condition = bc_t::kutta; break;
                    }
                switch (hc)
                {
                    case 'n': body->heat_condition = hc_t::neglect; break;
                    case 'i': body->heat_condition = hc_t::isolate; break;
                    case 't': body->heat_condition = hc_t::const_t; break;
                    case 'w': body->heat_condition = hc_t::const_w; break;
                }

                body->alist.push_back(att);
            }
            body->doUpdateSegments();
            body->doFillProperties();
        }
        else {
            fprintf(stderr, "S->Load(): ignoring field \"%s\"\n", comment);
            exit(-1);
        }
    }

    EnumerateBodies();

    return;
}
