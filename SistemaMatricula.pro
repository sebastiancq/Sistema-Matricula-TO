QT += widgets
CONFIG += c++17
CONFIG -= debug_and_release
TARGET = SistemaMatricula
TEMPLATE = app

INCLUDEPATH += include

HEADERS += \
    include/modelo/Persona.h \
    include/modelo/Estudiante.h \
    include/modelo/Docente.h \
    include/modelo/Administrativo.h \
    include/modelo/PersonaFactory.h \
    include/modelo/Horario.h \
    include/modelo/Aula.h \
    include/modelo/Cola.h \
    include/modelo/Curso.h \
    include/modelo/Matricula.h \
    include/modelo/ReglaMatricula.h \
    include/modelo/Excepciones.h \
    include/modelo/FuncionesGenericas.h \
    include/modelo/SistemaMatricula.h \
    include/modelo/RepositorioCsv.h \
    include/vista/VistaPrincipal.h \
    include/vista/DialogoDetalle.h \
    include/controlador/ControladorMatricula.h

SOURCES += \
    src/main.cpp \
    src/modelo/Persona.cpp \
    src/modelo/PersonaFactory.cpp \
    src/modelo/Horario.cpp \
    src/modelo/Curso.cpp \
    src/modelo/ReglaMatricula.cpp \
    src/modelo/SistemaMatricula.cpp \
    src/modelo/RepositorioCsv.cpp \
    src/vista/VistaPrincipal.cpp \
    src/vista/DialogoDetalle.cpp \
    src/controlador/ControladorMatricula.cpp

RESOURCES += recursos/recursos.qrc
