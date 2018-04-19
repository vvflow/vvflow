## Installation

Программный комплекс устанавливается из deb-репозитория:
```
# sudo apt-get install curl apt-transport-https
TOKEN=read_token # https://packagecloud.io/rosik/vvflow/token/default
echo "deb https://$TOKEN:@packagecloud.io/rosik/vvflow/ubuntu/ xenial main" | sudo tee /etc/apt/sources.list.d/vvflow.list
curl -sL "https://$TOKEN:@packagecloud.io/rosik/vvflow/gpgkey" | sudo apt-key add -
sudo apt-get update
sudo apt-get install vvflow
```

## Flow simulation

### *vvcompose*

Для того, что бы что-то посчитать, первым делом нужно
создать файл расчёта и описать моделируемое пространство `Space`.
Этим занимается программа `vvcompose`.
Полную информацию о синтаксисе можно найти в мануале `man vvcompose`.
Если коротко, то `vvcompose` представляет из себя
интерпретатор [lua](https://learnxinyminutes.com/docs/lua/).

Например вот так выглядит расчет цилиндра:

```
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

Геометрию тела можно не загружать из текстового файла,
а сразу генерировать внутри `vvcompose`.
Обо всём этом написано в мануале, поэтому обязательно его прочитайте: `man vvcompose`.

### *vvflow*

Следующим этапом является непосредственно запуск расчёта.
Этим занимается программа `vvflow`.
Если не углубляться в подробности, то всё выглядит просто:
```
vvflow --progress --profile re600_n350.h5
```

У команды `vvflow` есть две полезные опции:

 - `--progress`: печатать в консоль текущее время расчета. Это позволяет следить за процессом.
 - `--profile`: сохранять в файл stepdata распределение давления и трения по поверхности тела.
 Эта информация значительно увеличивает размер файла stepdata, поэтому без необходимости её лучше не включать.

Кулеры зажужжат, в текущей директории начнут появляться результаты, и через денёк другой можно будет заняться обработкой.
Если перспектива ждать не радует, а под рукой есть кластер c [PBS](https://en.wikipedia.org/wiki/Portable_Batch_System),
то запуск будет немного сложнее:

```
qsub -d. -l nodes=1:ppn=6 -N testrun <<EOF
    export PATH="/home/user/.local/bin";
    export LD_LIBRARY_PATH="/home/user/.local/lib";
    ulimit -c unlimited;
    export OMP_NUM_THREADS=6;
    exec vvflow re600_n350.h5;
EOF
```

### Postprocessing results

После того, как расчёт завершится,
в текущей директории будет лежать файл stepdata_re600_n350.h5
и огромное количество результатов в поддиректории results_re600_n350.

К счастью, в комплекте с комплексом есть несколько утилит,
призванных упростить жизнь при обработке полученных результатов:
* [vvxtract](#vvxtract)
* [vvplot, vvencode](#vvplot-vvencode)
* [vvawk](#vvawk)
* [gpquick](#gpquick)

### *vvxtract*

`vvxtract` является прямой противоположностью программы `vvcompose`.
Актуальную справку по опциям можно получить в мануале: `man vvxtract` (не пренебрегайте им).

Применительно к файлам c пространством `Space` утилита выводит все атрибуты пространства
примерно в том виде, как они выглядят в `vvcompose`:

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

`vvxtract` можно также использовать для распаковки файлов `stepdata`:

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

Самое простое, что можно сделать - нарисвать вихревую картину течения, или даже мультфильм.
Для этих целей служит утилита `vvplot`.
Здесь я привожу маленький кусочек справки, а полный набор аргументов можно узнать командой `vvplot --help`

```
usage: vvplot [-h] [-b] [-g] [-H] [-i] [-p] [-P] [-s] [-v] [-V] [-w] [-f] [-n]
              [--nooverride] [--colorbox] [--timelabel] [--spring]
              [--blankbody N] -x XMIN XMAX [-y YMIN YMAX] [--size WxH]
              [--referenceframe {o,b,f}] [--streamlines {o,b,f}]
              [--pressure {s,o,b,f}] [--tree FILE] [--debug]
              input_file output_file

positional arguments:
  input_file            input file
  output_file           output file or directory

optional arguments:
  -h, --help            show this help message and exit
  -b                    plot body
  -g                    plot vorticity
  -H                    plot heat particles
  -i                    plot ink (streaklines)
  -p                    plot pressure field
  -P                    plot pressure isolines
  -s                    plot streamlines
  -v                    plot vortex domains with dots
  -V                    plot vortex domains in bold
  -w, --grayscale       plot in grayscale
  --timelabel           draw time label in top left corner
  --spring              draw body spring
  --blankbody N         do not fill body (numeration starts with 1)
```

Пару слов о размерах картинки. `vvplot` всегда соблюдает масштаб по осям. Пользователь волен сам задавать два из трех параметров:

 - `-x XMIN XMAX`: границы картинки по оси абсцисс; допускается использовать одинаковые значения (`XMIN==XMAX`), в таком случае границы будут расчитаны автоматически, причем симметрично относительно прямой `X=XMIN`.
 - `-y YMIN YMAX`: границы картинки по оси ординат; аналогично можно задавать `YMIN==YMAX`.
 - `--size WxH`: размер картинки в пикселях; один из двух параметров можно не писать (например `--size 1280x` - высота изображения будет вычислена автоматически).

Примеры использования:

самый простой вариант
```
vvplot re600_n350.h5 -b -x -2 20 ./
```

немного красивее
```
vvplot results_re600_n350/010000.h5 -bvV -x -2 20 ./images --timelabel --spring
```

строим кадры для мультика
```
for f in results_re600_n350/*.h5; do vvplot $f -bvV -x -2 20 ./images --timelabel --spring; done;
```

кодируем кадры в единый видеофайл (с использованием ffmpeg)
```
vvencode 'images/*.png' re600_n350.mp4
```

### *h5xtract*

`h5xtract` - отличный инструмент для извлечения данных из файла stepdata. Синтаксис не покажу, а покажу сразу пример использования.


### *vvawk*

`vvawk` это целая пачка скриптов на языке awk, которая хорошо дополняет собой утилиту `vvxtract`.
Для более подробного описания запускайте утилиты без аргументов.

 - `vvawk.avg`: вычисляет арифметическое среднее (average) от всей колонки.
 - `vvawk.sd`: вычисляет среднеквадратическое отклонение (standard deviation) от всей колонки.
 - `vvawk.zeros`: печатает нули функции. 
 - `vvawk.mavg`: вычисляет бегущее среднее (moving average) в колонке.
 - `vvawk.drv`: вычисляет производную (derivative).
 - `vvawk.ampl`: вычисляет амплитуду (amplitude) колебаний значений в колонке
 (разность между средним всех локальных минимумов и средним всех локальных максимумов).

### *gpquick*

`gpquick` - это ултилита для быстрого предпросмотра графиков.

Например, мы хотим посмотреть на силу сопротивления из расчета выше:
```
vvxtract stepdata_re600_n350.h5 time body00/force_hydro | gpquick --points > fx_raw.png
```

А если нам не понравилась "шуба" на графике, можно отфильтровать ее с помощью moving average:
```
vvxtract stepdata_re600_n350.h5 time body00/force_hydro | vvawk.mavg -v span=100 - | gpquick --lines > fx_mavg.png
```

## Development and compilation

Сборку проекта удобнее всего выполнять внутри докер контейнера.
Об установке самого докера можно почитать
на [официальном сайте](https://docs.docker.com/engine/installation/linux/ubuntu/#install-docker).
Запуск девелоперского окружения делается при помощи [docker-compose](https://docs.docker.com/compose/install/):

```bash
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
cpack
```

Протестировать его можно в другом чистом контейнере:
```
sudo docker-compose exec tester /bin/bash
```

В нём можно установить свеже собранный `deb`-пакет и потренироваться запускать отдельные утилиты:
```
apt update && apt install -y ./deb/vvflow-*.deb
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
* http://127.0.0.1:1208/utils/vvplot3/vvplot.1.html

## Links

* [ronn](http://ricostacruz.com/cheatsheets/ronn.html) - markdown to manpages cheatsheet
