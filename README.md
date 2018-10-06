## Installation

Platforms supported:

* debian jessie
* ubuntu xenial
* ubuntu bionic

Run in shell:

```
# sudo apt install curl apt-transport-https
curl -L https://packagecloud.io/vvflow/nightly/gpgkey | sudo apt-key add -
echo "deb https://packagecloud.io/vvflow/nightly/debian/ jessie main" | sudo tee /etc/apt/sources.list.d/vvflow.list
sudo apt update
sudo apt install vvflow
```

## Documentation

* [/usr/share/doc/vvflow/](file:///usr/share/doc/vvflow/)
* `man vvcompose`
* `man vvxtract`
* `man vvplot`

## Flow simulation

### *vvcompose*

Defining the CDF problem is handled by *vvcompose* tool.
It is a [lua](https://learnxinyminutes.com/docs/lua/) interpreter,
so it supports everything that lua supports, and even more.

Flow around circular cylinder:

```
#!/bin/bash
vvcompose <<EOF
    S.inf_vx = "1"
    S.re = 600
    S.dt = 0.005
    S.dt_save = 0.1
    S.finish = 500

    cyl = gen_cylinder{R=0.5, N=350}
    cyl.label = "cyl"
    S.body_list:insert(cyl)

    S.caption = "re600_n350"
    S:save(S.caption..".h5")
EOF
```

More information is given in `man vvcompose`.

### *vvflow*

The flow simulation itself is performed by *vvflow* tool:

```
vvflow --progress --profile re600_n350.h5
```

There is no manual entry for *vvflow*, but there is only two options:

 - `--progress`: print current internal time to stdout.
 - `--profile`: save pressure and friction profiles to file `stepdata`.

The *vvflow* can be run on a [PBS](https://en.wikipedia.org/wiki/Portable_Batch_System) cluster:

```
#!/bin/bash
qsub -d. -l nodes=1:ppn=6 -N testrun <<EOF
    export OMP_NUM_THREADS=6;
    exec vvflow re600_n350.h5;
EOF
```

### Postprocessing results

The results are saved to the current working directory during the simulation:
* `stepdata_re600_n350.h5` - integral parameters (forces, dispositions) time series.
* `results_re600_n350/*.h5` - vortex distributions over time.

Those are raw data, which can be processed using one of the following:
* [vvxtract](#vvxtract)
* [vvplot, vvencode](#vvplot-vvencode)
* [vvawk](#vvawk)
* [gpquick](#gpquick)

### *vvxtract*

The *vvxtract* tool is the opposite to the *vvcompose*.
It prints `.h5` file content in human readable form.

```
$ vvxtract re600_n350.h5
-- space
S.re = 600
S.inf_vx = '1'
S.time = '0/1' -- 0
S.dt = '5/1000' -- 0.005
S.dt_save = '1/10'
S.finish = 500
-- #S.body_list = 1 -- number of bodies
-- #S.vort_list = 0 -- number of vortex domains
-- #S.sink_list = 0 -- number of sinks and sources

-- body00 (cyl)
cyl.label = 'cyl'
cyl.slip = false -- no-slip
-- #cyl = 350 -- number of segments
-- cyl.get_axis() = {0, 0} -- rotation axis
-- cyl.get_cofm() = {0, 0} -- center of mass
-- cyl.get_slen() = 3.14155 -- surface length
-- cyl.get_area() = 0.785356
-- cyl.get_moi_cofm() = 0.0981642
-- cyl.get_moi_axis() = 0.0981642
```

Stepdata files can be handled the same way:

```
$ vvxtract stepdata_re600_n350.h5 time body00/force_hydro | less
#time   body00/force_hydro[1]   body00/force_hydro[2]   body00/force_hydro[3]
+0.000000e+00   +3.140723e+02   -2.296381e-13   +5.088693e-15
+5.000000e-03   +2.557315e+00   +1.390750e-04   -1.982764e-14
+1.000000e-02   -1.157489e+00   +4.054699e-04   +1.064268e-04
+1.500000e-02   +8.585840e-01   -1.463448e-01   -6.041857e-02
...
```

### *vvplot, vvencode*

To plot the images use *vvplot* tool:

```
vvplot re600_n350.h5 -B -x -2,20 ./
```

```
vvplot results_re600_n350/010000.h5 -BV -x -2,20 ./images --spring
```

Draw animation:
```
ls results_re600_n350/*.h5 | xargs -I{} vvplot {} -BV -x -2,20 ./images --spring
```

Encode video using ffmeg
```
vvencode 'images/*.png' re600_n350.mp4
```

### *vvawk*

The *vvawk* tool is a bunch of helpful awk scripts, like command-line MS-Excel.
It can be very useful in combination with *vvxtract*.

 - `vvawk.avg`: the arithmetic mean in a column.
 - `vvawk.sd`: the standard deviation in a column.
 - `vvawk.zeros`: find zeroes.
 - `vvawk.mavg`: the moving average in a column.
 - `vvawk.drv`: the derivative.
 - `vvawk.ampl`: the amplitude in a column - avg(max) - avg(min).

### *gpquick*

The `gpquick` tool is used to preview plots.

```
vvxtract stepdata_re600_n350.h5 time body00/force_hydro | gpquick --points > fx_raw.png
```

In combination with *vvawk*:
```
vvxtract stepdata_re600_n350.h5 time body00/force_hydro | vvawk.mavg -v span=100 - | gpquick --lines > fx_mavg.png
```

## Development and compilation

Сборку проекта удобнее всего выполнять внутри докер контейнера.
Об установке самого докера можно почитать
на [официальном сайте](https://docs.docker.com/engine/installation/linux/ubuntu/#install-docker).
Запуск девелоперского окружения делается при помощи [docker-compose](https://docs.docker.com/compose/install/):

```bash
export OS=ubuntu-artful
sudo docker-compose build --pull
sudo docker-compose up --no-start
sudo docker-compose start
sudo docker-compose exec builder /bin/bash
```

Внутри контейнера присутствуют все необходимые девелоперские либы (см. `Dockerfile`).
Исходники vvflow примаунчены к директории `/vvflow`.
Для сборки проекта используется `cmake`:

```
cmake -D CMAKE_BUILD_TYPE=Release /vvflow
make -j
make test
cpack
```

Протестировать его можно в другом чистом контейнере:
```
sudo docker-compose exec tester /bin/bash
```

В нём можно установить свеже собранный `deb`-пакет и потренироваться запускать отдельные утилиты:
```
apt update && apt install -y ./deb/vvflow-*.deb
find ./tests -type f -exec {} \;
vvcompose
```

Распространять скомпилированный проект можно с помощью deb-пакетов через [packagecloud.io](https://packagecloud.io/):

```
export PACKAGECLOUD_TOKEN=api_token # https://packagecloud.io/api_token
pkgcloud-push rosik/vvflow/ubuntu/xenial ./vvflow.deb
```

В процессе разработки также может пригодиться предпросмотр Markdown файлов:

```
grip /vvflow 0.0.0.0:1207
```

* README.md: http://127.0.0.1:1207

Документацию можно проверять командой `man` внутри контейнера:
```
man -l utils/vvcompose/vvcompose.1
```
Либо через http сервер:
```
python -m SimpleHTTPServer 1208
```
* http://127.0.0.1:1208/utils/vvcompose/vvcompose.1.html
* http://127.0.0.1:1208/utils/vvxtract/vvxtract.1.html
* http://127.0.0.1:1208/utils/vvplot/vvplot.1.html

## Links

* [ronn](http://ricostacruz.com/cheatsheets/ronn.html) - markdown to manpages cheatsheet
