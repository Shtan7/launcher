You can find english project description below russian one.

# Launcher: RU

## Краткое описание.

Данный проект - это простое клиент - серверное приложение, написанное с помощью boost asio. Оно обладает следующими функциями и свойствами:

- Для передачи данных задействован TLS протокол.
- Сервер использует асинхронную модель исполнения, основанную на корутинах из c++20.
- Реализована система аккаунтов.
- Данные учетных записей пользователей хранятся при помощи базы данных postgre. Весь интерфейс для взаимодействия с postgresql является асинхронным и полностью интегрированным в систему экзекьюторов boost asio.
- Реализована система обновления клиентских файлов.

Для теста функции обновления файлов создайте возле исполняемого файла сервера директорию **data**. При выборе опции **update** в клиенте, приложение будет синхронизировать состояние своих файлов с состоянием файлов сервера. 

## Инструкции по сборке.

Для сборки клиента и сервера вам понадобится CMake, а так же Conan.

> Данные действия необходимо отдельно выполнить для клиента и сервера.

В начале вам понадобится создать директорию 'build':

```
$ mkdir build
```

После потребуется воспользоваться Conan'ом для загрузки необходимых зависимостей:

```
$ cd build
$ conan install ..
```

Так же возможен сценарий, когда под вашу ОС не нашлось prebuilt package. В таком случае нужно добавить один параметр:

```
$ conan install build=missing ..
```

Если вы пользователь Linux'а, то выполнять конан инсталл нужно следующей командой:

```
$ conan install --settings compiler.libcxx="libstdc++11" ..
```

Далее необходимо выполнить CMake скрипт:

```
$ cmake ..
```

После генерации файлов нужной вам билдсистемы вам остается только воспользоваться ими и собрать проект. Так же присутствует возможность совершить это через команду CMake:

```
$ cmake --build .
```

Так же вам потребуется база данных postgresql, в которой необходимо будет настроить пользователя, создать инстанс базы данных, а так же таблицу с полями 'логин', 'пароль' для хранения клиентских данных. 

В качестве альтернативы действия выше вы можете отнаследовать базовый абстрактный класс базы данных и реализовать свой интерфейс для любой другой понравившейся вам базы данных. Единственной проблемой при данном варианте будет реализация асинхронного интерфейса и его интеграция с boost asio.

Все данные для подключения к postgresql читаются из файла **connection_data.txt**.

## Генерация файлов, необходимых для работы TLS протокола.

Для теста вы можете использовать предоставленные ключи, сертификаты или же сгененировать свои. Для генерации вам понадобится собрать openssl, установить environment variable **OPENSSL_CONF**, которая будет указывать на файл **openssl.conf** и исполнить следующую комбинацию команд:

```
$ openssl genrsa -out rootca.key 2048
$ openssl req -x509 -new -nodes -key rootca.key -days 20000 -out rootca.crt
$ openssl genrsa -out user.key 2048
$ openssl req -new -key user.key -out user.csr
$ openssl x509 -req -in user.csr -CA rootca.crt -CAkey rootca.key -CAcreateserial -out user.crt -days 20000
$ openssl dhparam -out dh2048.pem 2048
```

При создании сертификатов вас будут просить ввести данные и важно, чтобы у обоих сертификатов различалось поле **Common Name**. Для проверки валидности сгенерированных данных используйте следующие команды:

```
$ openssl verify -CAfile rootca.crt rootca.crt
$ openssl verify -CAfile rootca.crt user.crt
$ openssl verify -CAfile user.crt user.crt
```

Первые две команды должны вернуть положительный результат, а последняя провалиться. Если последняя команда возвращает статус **ОК**, то вероятнее всего вы ввели одинаковые **Common Name** у обоих сертификатов.

После генерации и успешных тестов поместите **user.crt**, **user.key**, **dh2048.pem** в родительскую директорию исполняемых файлов. Для лаунчера понадобится **rootca.crt**. Все остальные файлы уберите подальше.

### P.S.

Вы можете найти в хидерах xml комментарии для каждого метода \ функции.

# Launcher: EN

## Brief description

This project is a simple client-server application written with boost asio library. It has the following features:

- Data transfer using the TLS protocol.
- The server uses an asynchronous execution model based on coroutines from c++20.
- The account system is implemented.
- The postgre database is used to store user data. The entire database interface is asynchronous and built into the boost executors system.
- The file update system is implemented.

If you want to test the file update feature then you need to create a **data** directory near the server executable file and put something inside. When you select the **update** function in the client the client files will be synchronized with the server.

## Build instructions

Firstly you need to install CMake and Conan. 

> These steps must be performed separately for the client and server.

At the beginning you need to create directory 'build':

```
$ mkdir build
```

Then you need to use Conan to download the required dependencies:

```
$ cd build
$ conan install ..
```

It is possible that there is no prebuilt package for your OS. In that case you need to execute command with one additional parameter:

```
$ conan install build=missing ..
```

If your OS is Linux you need to perform conan installation differently:

```
$ conan install --settings compiler.libcxx="libstdc++11" ..
```

Next, execute the CMake script:

```
$ cmake ..
```

After build system files generation you can use generated files to build the project or you can use the following CMake command:

```
$ cmake --build .
```

You also need a postgresql database in which you need to configure a user, create a database instance, as well as a table with fields 'login', 'password' to store client data. Or you can inherit abstact database class and implement your own interface. But there is a problem. In this case you have to implement the asynchronous interface that built into the boost executors system.

All data for connecting to postgresql is read from the **connection_data.txt** file.

## Generation of files required for the TLS protocol.

For testing purposes you can use the files already provided. Or you can generate your own files. To generate you need to build openssl and set environment variable **OPENSSL_CONF** that points to **openssl.conf** file. Then you need to execute the following commands: 

```
$ openssl genrsa -out rootca.key 2048
$ openssl req -x509 -new -nodes -key rootca.key -days 20000 -out rootca.crt
$ openssl genrsa -out user.key 2048
$ openssl req -new -key user.key -out user.csr
$ openssl x509 -req -in user.csr -CA rootca.crt -CAkey rootca.key -CAcreateserial -out user.crt -days 20000
$ openssl dhparam -out dh2048.pem 2048
```

When creating certificates, you will be prompted for personal information and it is important that both certificates have different **Common Name** field. To test the generation execute the following commands:

```
$ openssl verify -CAfile rootca.crt rootca.crt
$ openssl verify -CAfile rootca.crt user.crt
$ openssl verify -CAfile user.crt user.crt
```

The first two commands must return positive result and the last one must to fail. If you get different result then you did something wrong. Try to repeat a sequence and make sure that **Common Name** fields are different.

After generation and successful tests put **user.crt**, **user.key**, **dh2048.pem**, **rootca.crt** in parent directory of executable files. You don't need the rest of the files. 

### P.S.

You can find xml comments for each method \ function in header files.