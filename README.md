# Audio Volume Normal
Иногда, особенно на торрентах, встречаются медиа файлы просто с низким уровнем громкости (-5dB -10dB).
Иногда это связано с тем что аудио поток (дорожка) записан в формате 5.1 и там аудио информация
распределена между пятью каналами и при прослушивании на "обычном стерео" устройстве (телевизор)
информация для двух каналов не полная и звук получается тихий.
Программа может преобразовать формат аудио из 5.1 (6.1, 7.1 ...) в стандартые stereo (2 канала).
Программа увеличивает уровень громкости в медиафайлах формата avi, mkv, mpeg4, ts до нормы.
Норма это уровень 0дБ, по ощущениям как уровень звука в телевизионной трансляции.

[Скачать последнюю версиюможно здесь](https://github.com/Vladimir-N-sk/AudioVolumeNormal/releases/latest)

## Содержание
- [Использование](#использование)
- [Технологии](#технологии)
- [Разработка](#разработка)
- [В планах](#в-планах)
- [Автор проекта](#автор-проекта)

## Использование
Проект разработан для использования на платформах Windows и Linux. Разработан только для 64-х битных версий.
Тестировался на Windows 7, 10, 11 и Linux Ubuntu 18.04.
Программа не требует какой-либо установки. Просто распаковываем архив в отдельную папку.
Запускаемый файл AudioVolumeNormal(.exe)

После запуска программы выбираем папку где лежат медиафайлы.  Кнопка "Найти файлы" выводит список
файлов на экран. Выбор файлов или даблк-клик на соответствующей строке или выделение списком с помощью
клавиш Ctrl или Shift.

### Выбор задания
#### *Узнать данные аудио*
Программа сканирует выбранный файл и вычисляет максимальный уровень аудио в файле. Далее эта информация, а также информация об
аудио кодеке файла отображается на экране. Программа сканирует только три первых аудио потока (если они есть).
Обычно это "русский цензурный", "русский альтернативный" и оригинальный язык.

#### *Изменить уровень аудио*
Если сразу выбрать задание на изменение уровня аудио, то сначала будет произведено сканирование выбранного файла на предмет
нахождения максимального уровня, затем программа перейдет к изменению уровня в файле.
Изменения будут по такому алгоритму, допустим в файле три аудио дорожки (потока) с уровнем -10, -6 и - 2 дБ.
Программа выберет значение -2дб и будет увеличивать уровень во всех 3-х аудио потоках на 2 дБ. В итоге в файле будет
-8дБ, -6дБ и 0дб. Тут действует два ограничения, во 1-х изменяются уровни сигнала одновременно во всех выбранных потоках и
на уровень более 0дБ нельзя увеличить аудио сигнал в файле. Поэтому, если изначально хотя бы один аудио сигнал имеет
уровень от 0 до -1 дБ изменения в файле произведены не будут.
НО! Если на первом шаге сначала выполнить задание "узнать уровень аудио" и снять галочку выбора с какого либо аудио потока
то он не будет участвовать в определении максимального уровня сигнала, но и в выходной файл не попадет. Таки образом, из примера, если убрать галочку выбора с аудио -2дБ в итоговом файле будет две аудио дорожки с уровнями -4дБ и 0дБ.
Выходной файл будет иметь имя AVN_староей имя и будет расположен рядом с оригиналом, в той же папке.

#### *Изменить формат 5.1 в stereo*
Для изменения формата аудио необходимо сначала выполнить пункт "Узнать данные аудио". Далее необходимо оставить "галочки"
выбора на тех аудио потоках (дорожках) которые мы хотим оставить в выходном файле. Если снять "галочку" выбора, то этот аудио
поток проигнорируется и не попадет в выходной файл. Выходной будет начинаться с префикса 5t2_(далее имя входного файла) и будет
располагаться в той же папке, что и исходный файл.


## Технологии
- [FFmpeg](https://ffmpeg.org/)
- [Qt Framework](https://www.qt.io/product/framework)


## Разработка

### Требования
Для установки и компиляции проекта, необходим [Qt Creator](https://www.qt.io/product/development-tools).

### Установка зависимостей
Для работы программы используется слегка доработанная версия ffmpeg.
[Взять исходные файлы ffmpeg можно тут] (https://ffmpeg.org/download.html)

В файле FFmpeg/fftools/ffmpeg.c

меняем
```c
const char end = is_last_report ? '\n' : '\r';
```
на
```c
const char end = is_last_report ? '\n' : '\n';
```
и выполняем компиляцию проекта ffmpeg.
Размещаем вновь полученный файл в поддериктории проекта /lib с именем libffmpeg, чтобы не путать с оригинальным ffmpeg.

## В планах
- [x] Добавить другие форматы медиафайлов
- [ ] ...

## Автор проекта

- [Vladimir-N-sk](alvladnik@gmail.com) — Программист-любитель
