VVFlow. Инструкция по установке
=======

Компиляция
----------

Сборку проекта из исходников следует проводить внутри докер контейнера.
Об установке самого докера можно почитать на [официальном сайте](https://docs.docker.com/engine/installation/linux/ubuntu/#install-docker).
После этого следует собрать и запустить контейнер:
```
#!console
$ cd /path/to/vvflow
$ sudo docker build -t vvflow-build:latest ./
$ mkdir build
$ sudo docker run -it --rm \
    -v $PWD:/vvflow:ro \
    -v $PWD/build:/root \
    vvflow-build /bin/bash
```

Вы оказались внутри контейнера. Кроме вас здесь присутствуют все необходимые девелоперские либы (см. `Dockerfile`).
Исходники vvflow примаунчены к директории `/vvflow`. И если в `/root` заняться компиляцией, то все бинарники окажутся на host'е в директории `./build`.
Поэтому делаем так:

```
#!console
cd /root
cmake /vvflow
make -j
cpack
```

Установка
---------

На данный момент единственным вариантом установки комплекса является сборка из исходников.
Для этого используйте команду
```
#!console
$ make all
```

По умолчанию установка производится в домашней папке командой
```
#!console
$ make install
```

После этих манипуляций комплекс расползется по директориям

 - `~/.local/bin` - бинарники и скрипты
 - `~/.local/lib` - библиотеки
 - `~/.local/share/vvhd` - автодополнение bash

Так как комплекс устанавливается в непривычные для системы места необходимо подправить конфиги. Добавьте следующие строчки в конец файла `~/.bashrc`

```
#!bash
export PATH=$HOME/.local/bin:$PATH
export LD_LIBRARY_PATH=$HOME/.local/lib/:$LD_LIBRARY_PATH
[[ -f $HOME/.local/share/vvhd/bash_completion ]] && source $HOME/.local/share/vvhd/bash_completion
```

И перезагрузите его командой
```
#!console
$ source ~/.bashrc
```

Использование
-------------
