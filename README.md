QtMail is a library that ...

The original source files are extracted from libqxt library SHA: 9068e83def488d9e3bcdb734cf242ce64c024214
 
## Getting Started

### Usage(1): Use Mail as Qt5's addon module

* Download the source code.

* Put the source code in any directory you like. At the toplevel directory run

**Note**: Perl is needed.

```
    qmake
    make
    make install
```

The library, the header files, and others will be installed to your system.

* Add following line to your qmake's project file:

```
    QT += mail
```

* Then, using Qt Mail in your code

### Usage(2): Use source code directly

The package contains a **qtmail.pri** file that allows you to integrate the component into applications that use qmake for the build step.

* Download the source code.

* Put the source code in any directory you like. For example, 3rdparty:

```
    |-- project.pro
    |-- ....
    |-- 3rdparty\
    |     |-- qtmail\
    |     |
```

* Add following line to your qmake project file:

```
    include(3rdparty/qtmail/src/mail/qtmail.pri)
```

**Note**: If you like, you can copy all files from *src/mail* to your application's source path. Then add following line to your project file:

```
    include(qtmail.pri)
```

* Then, using Qt Mail in your code

## References

* https://libqxt.bitbucket.org
* https://qt.gitorious.org/qt-labs/messagingframework
* https://github.com/bluetiger9/SmtpClient-for-Qt
* https://github.com/nicholassmith/Qt-SMTP
* http://morf.lv/modules.php?name=tutorials&lasit=20
