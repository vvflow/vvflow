Установка программного комплекса
=======

Программный комплекс устанавливается из deb-репозитория:
```
#!bash
# sudo apt-get install curl apt-transport-https
TOKEN=c924a03ddb1308dfdd423f9735693041557bdb3300138134
echo "deb https://$TOKEN:@packagecloud.io/rosik/vvflow/ubuntu/ xenial main" | sudo tee /etc/apt/sources.list.d/vvflow.list
curl -sL "https://$TOKEN:@packagecloud.io/rosik/vvflow/gpgkey" | sudo apt-key add -
sudo apt-get update
sudo apt-get install vvflow
```

Проведение вычислительного эксперимента
=======

*vvcompose*
-----------

Для того, что бы что-то посчитать, первым дело нужно создать файл расчета. Этим занимается программа `vvcompose`. Полный набор опций можно узнать, если запустить ее без аргументов.

```
#!text
usage: vvcompose COMMAND ARGS [...]

COMMAND:
    load {hdf|body|vort|heat|ink|ink_source} FILE: load data from txt file 
    save FILE: save space in hdf5 format
    del {bodyXX|vort|heat|ink|ink_source}: remove specific list from space
    set VARIABLE VALUE: set given parameter in space
```

Для команды `load` понадобится подготовленный файл с данными. Формат файла зависит от того, что мы собираемся загружать.

 - `hdf`: тот самый файл расчета, предварительно созданный при помощи `vvcompose` или сохраненный при расчете программой `vvflow`
 - `body`: простой текстовый файл, в две колонки `x y`, координаты вершин тела
 - `vort`: простой текстовый файл, в три колонки `x y g`, третья колонка соответствует циркуляции вихрей
 - `ink`, `ink_source`: простой текстовый файл, в три колонки `x y id`, третья колонка на результатах расчета не сказывается, но может пригодиться при обработке результатов

Вот так выглядит расчет цилиндра

```
#!bash
vvcompose \
    set caption re600_n350 \
    load body cyl350.txt \
    set inf_speed.x '1' \
    set dt 0.005 \
    set dt_save 0.1 \
    set re 600 \
    set time 0 \
    set time_to_finish 500 \
    save re600_n350.h5
```

Вместо того, что бы загружать тело из файла, можно соорудить удобный костылик на баше
```
#!bash
function gen_cylinder() {
awk -v N=$1 -f - /dev/null <<AWK
    BEGIN{pi=atan2(0, -1)}
    END{
        for(i=0;i<N;i++) {
            x =  0.5*cos(2*pi*i/N)
            y = -0.5*sin(2*pi*i/N)
            printf("%+0.5f %+0.5f\n", x, y)
        }
    }
AWK
}

vvcompose load body <(gen_cylinder 350)
```

Аналогичным способом можно генерировать и другие данные. За примерами стоит заглянуть в папку с тестами. Там можно найти много интересного.

*vvflow*
--------

Следующим этапом является непосредственно запуск расчета. Расчетом занимается программа `vvflow`. Если не углубляться в подробности, то все выглядит просто:
```
#!bash
vvflow --progress --profile re600_n350.h5
```

У команды `vvflow` есть две полезные опции:

 - `--progress`: печатать в консоль текущее время расчета. Это позволяет следить за процессом.
 - `--profile`: сохранять в файл stepdata распределение давления и трения по поверхности тела. Эта информация значительно увеличивает размер файла stepdata, поэтому без необходимости ее лучше не включать.

Кулеры зажужжат, в текущей директории начнут появляться результаты, и через денёк другой можно будет заняться обработкой. Если перспектива ждать не радует, а под рукой есть кластер c [PBS](https://en.wikipedia.org/wiki/Portable_Batch_System), то запуск будет немного сложнее.

```
#!bash
cat <<EOF | qsub -d. -l nodes=1:ppn=6 -N testrun
    export PATH="/home/user/.local/bin";
    export LD_LIBRARY_PATH="/home/user/.local/lib";
    ulimit -c unlimited;
    export OMP_NUM_THREADS=6;
    exec vvflow re600_n350.h5;
EOF
```

Обработка результатов
---------------------

После того, как расчет завершится, мы обнаружим в текущей директории файл stepdata_re600_n350.h5 и огромное количество результатов в поддиректории results_re600_n350. Логичным будет вопрос - а что с этими гигабайтами делать?
К счастью, в комплекте с комплексом есть несколько скриптов, призванных упростить жизнь при обработке полученных результатов. С их помощью обработку целой серии расчетов можно свести к парочке вложенных циклов.

*vvplot, vvencode*
------------------

Самое простое, что можно сделать - нарисвать вихревую картину течения, или даже мультфильм. Для этих целей служит утилита `vvplot`.
Здесь я привожу маленький кусочек справки, а полный набор аргументов можно узнать командой `vvplot --help`

```
#!text
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
#!bash
vvplot re600_n350.h5 -b -x -2 20 ./
```

немного красивее
```
#!bash
vvplot results_re600_n350/010000.h5 -bvV -x -2 20 ./images --timelabel --spring
```

строим кадры для мультика
```
#!bash
for f in results_re600_n350/*.h5; do vvplot $f -bvV -x -2 20 ./images --timelabel --spring; done;
```

кодируем кадры в единый видеофайл (с использованием ffmpeg)
```
#!bash
vvencode 'images/*.png' re600_n350.mp4
```

*h5xtract*
----------

`h5xtract` - отличный инструмент для извлечения данных из файла stepdata. Синтаксис не покажу, а покажу сразу пример использования.
```
#!console
$ h5xtract stepdata_re600_n350.h5 time body00/force_hydro
time    body00/force_hydro
0       5.2913672      15.104421       -3.8444971
0.01    1.4640913      -1.5818661      2.0715502
0.02    1.3909379      -1.0275889      1.7571091
0.03    1.3555557      -1.4139547      1.8501545
...
```

*vvawk*
-------

`vvawk` это целая пачка скриптов на языке awk, которая хорошо дополняет собой `h5xtract`.
Для более подробного описания запускайте утилиты без аргументов.

 - `vvawk.avg`: вычисляет арифметическое среднее (average) от всей колонки.
 - `vvawk.sd`: вычисляет среднеквадратическое отклонение (standard deviation) от всей колонки.
 - `vvawk.zeros`: печатает нули функции. 
 - `vvawk.mavg`: вычисляет бегущее среднее (moving average) в колонке.

*gpquick*
---------

`gpquick` - это ултилита для быстрого предпросмотра простецких графиков.

Например, мы хотим посмотреть на силу сопротивления из расчета выше:
```
#!bash
h5xtract stepdata_re600_n350.h5 time body00/force_hydro | gpquick --points > fx_raw.png
```

А если нам не понравилась "шуба" на графике, можно отфильтровать ее с помощью moving average:
```
#!bash
h5xtract stepdata_re600_n350.h5 time body00/force_hydro | vvawk.mavg -v span=100 - | gpquick --lines > fx_mavg.png
```

Разработка 
=======

Сборку проекта удобнее всего выполнять внутри докер контейнера.
Об установке самого докера можно почитать на [официальном сайте](https://docs.docker.com/engine/installation/linux/ubuntu/#install-docker).
Для запуска контейнера удобнее используется `buildenv.sh` из репозитория:
```
#!bash
. buildenv.sh
# в качестве аргумента можно указать целевой дистрибутив
# . buildenv.sh ubuntu-xenial # default
# . buildenv.sh debian-jessie
```

Внутри контейнера присутствуют все необходимые девелоперские либы (см. `Dockerfile`).
Исходники vvflow примаунчены к директории `/vvflow`. И если в `/root` заняться компиляцией, то все бинарники окажутся на host'е в директории `./build`.

Для сборки проекта используется cmake:

```
#!bash
cd /root
cmake -D CMAKE_BUILD_TYPE=Release /vvflow
make -j
cpack
```

Распространять скомпилированный проект можно с помощью deb-пакетов через [packagecloud.io](https://packagecloud.io/):
```
#!bash
export PACKAGECLOUD_TOKEN=api_token # https://packagecloud.io/api_token
pkgcloud-push rosik/vvflow/ubuntu/xenial ./vvflow.deb
```

В процессе разработки также может пригодиться предпросмотр Markdown файлов:
```
#!bash
grip /vvflow 0.0.0.0:1207
```
* README.md: http://127.0.0.1:1207
* INSTALL.md: http://127.0.0.1:1207/INSTALL.md

Документацию можно проверять командой `man` внутри контейнера:
```
#!bash
man -l utils/vvcompose/vvcompose.1
```
Либо через http сервер:
```
#!bash
python -m SimpleHTTPServer 1207
```
* http://127.0.0.1:1207/utils/vvcompose/vvcompose.1.html
