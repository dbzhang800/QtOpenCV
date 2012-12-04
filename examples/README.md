## Configure QtOpenCV

### Install to non-standard location

If OpenCV2 does not installed in the standard directory, it is easy too.

 * set qmake variables to tell qmake or QtCreator [note: you only need to do this one time]

```
   qmake -set OPENCV_VERSION 2.4.3
   qmake -set OPENCV_INCPATH D:/opencv/build/include
   qmake -set OPENCV_LIBPATH D:/opencv/build/x86/vc10/lib
```

or 

 * set system environment variables to tell qmake or QtCreator.

```
   set OPENCV_VERSION=2.4.3
   set OPENCV_INCPATH=D:/opencv/build/include
   set OPENCV_LIBPATH=D:/opencv/build/x86/vc10/lib
```

using follow formats if you are using bash or csh.

```
export VARIABLE=value
setenv VARIABLE value
```

### Install to standard location

[For windows user] 
Header files can be put in `%QTDIR%/include/opencv2/` and libraries can be copied to `%QTDIR%/lib`

[For linux user]
Nothing need to do if you OpenCV installed through the system package manager.

