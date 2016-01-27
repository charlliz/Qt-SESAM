# Qt-SESAM

**SESAM - Super Easy & Secure Authentication Management**

Qt-SESAM is a user-friendly application that enables you to generate strong passwords that you can use for all services you are using. The passwords are generated in realtime from service name, user name, a randomly shuffled salt and the master password. For services which don't allow passwords to be changed (e.g. credit cards) Qt-SESAM can also store fixed passwords.

Qt-SESAM has a unique feature which lets you choose the complexity and length of the password in a colored widget:

![EasySelectorWidget](https://raw.githubusercontent.com/ola-ct/Qt-SESAM/master/doc/qt-sesam-screenshot.png)

The widget tells you how long the [Tianhe-2 supercomputer](https://en.wikipedia.org/wiki/Tianhe-2) and your computer would need to crack a password with the selected settings.

You can share Qt-SESAM's settings across your computers via a [dedicated synchronisation server](https://github.com/ola-ct/ctSESAM-server) and a file located on a cloud drive like [OwnCloud](https://owncloud.org/), [Google Drive](https://www.google.com/drive/), [Microsoft OneDrive](https://onedrive.live.com/about/) or [Dropbox](https://www.dropbox.com/).

This is secure because all of your settings are [AES](https://en.wikipedia.org/wiki/Advanced_Encryption_Standard)-encrypted with a 256 bit long [key](https://en.wikipedia.org/wiki/Key_(cryptography)) and a 128 bit long [IV](https://en.wikipedia.org/wiki/Initialization_vector) derived from your master password with [PBKDF2](https://en.wikipedia.org/wiki/PBKDF2).

Qt-SESAM supports Windows, Linux and Mac OS X. An [Android app](https://github.com/pinae/ctSESAM-android) compatible to Qt-SESAM is to be advancing.

## Download

 * [Source code](https://github.com/ola-ct/Qt-SESAM)
 * [Binaries for Windows and OS X](https://github.com/ola-ct/Qt-SESAM/releases)

## Important infos

 * [FAQ in German](https://github.com/ola-ct/Qt-SESAM/wiki/FAQ-%5Bde%5D)
 * [Build instructions for Linux](https://github.com/ola-ct/Qt-SESAM/wiki/Build-for-Linux)
 * [Build instructions for Windows](https://github.com/ola-ct/Qt-SESAM/wiki/Build-for-Windows)
 * [How to contribute to Qt-SESAM](https://github.com/ola-ct/Qt-SESAM/wiki/Contribute)

## Further reading

In German:

 * [Oliver Lau, Sesam, synce dich!, Den c't-SESAM-Synchronisierungsserver installieren, c't 23/15, S. 182]()
 * [Oliver Lau, Sesam, öffne dich!, Passwörter verwalten mit c't SESAM, c't 20/15, S. 182](http://www.heise.de/ct/ausgabe/2015-20-Passwoerter-verwalten-mit-c-t-SESAM-2793210.html)
 * [Oliver Lau, Aus kurz wird lang, Passwörter sicher speichern mit Hilfe von PBKDF2, c't 17/15, S. 180](http://heise.de/-2753149)
 * [Johannes Merkert, Schlangenbeschwörung für Einsteiger, Programmieren lernen mit Python, c't 18/15, S. 118](http://heise.de/-2767841)
 * [Jürgen Schmidt, Eines für alle, Ein neues Konzept für den Umgang mit Passwörtern, c't 18/14, S. 82](http://heise.de/-2284364)
