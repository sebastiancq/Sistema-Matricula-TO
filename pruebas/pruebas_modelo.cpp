// Comprueba las reglas de SistemaMatricula llamando a sus metodos. Se
// ejecuta desde la carpeta del proyecto para encontrar los archivos de datos/.
#include "modelo/Excepciones.h"
#include "modelo/PersonaFactory.h"
#include "modelo/RepositorioCsv.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

int superadas = 0;
int total = 0;

void verificar(bool condicion, const string& descripcion)
{
    total++;
    if (condicion)
        superadas++;
    cout << (condicion ? "Superada  " : "FALLIDA   ") << descripcion << '\n';
}

// Ejecuta la accion y comprueba que lance la excepcion esperada.
template <typename Error, typename Accion>
void verificarError(Accion accion, const string& descripcion)
{
    bool lanzo = false;
    try
    {
        accion();
    }
    catch (const Error&)
    {
        lanzo = true;
    }
    catch (const exception&)
    {
    }
    verificar(lanzo, descripcion);
}

void registrar(SistemaMatricula& s, const string& tipo, const string& nombre,
               const string& codigo, const string& dato)
{
    s.registrarPersona(
        PersonaFactory::crearPersona(tipo, nombre, codigo, dato));
}

Horario h(const string& texto)
{
    return Horario::desdeTexto(texto);
}

void agregarReglas(SistemaMatricula& s, int maximoCreditos)
{
    s.agregarRegla(make_unique<ReglaCarrera>());
    s.agregarRegla(make_unique<ReglaCruceHorario>());
    s.agregarRegla(make_unique<ReglaLimiteCreditos>(maximoCreditos));
    s.agregarRegla(make_unique<ReglaPrerrequisito>());
}

int main()
{
    registrarTiposDePersona();
    SistemaMatricula s;
    agregarReglas(s, 10);
    int avisosPersonas = 0;
    int avisosMatriculas = 0;
    s.agregarObservador(
        [&](Cambio cambio)
        {
            if (cambio == Cambio::Personas)
                avisosPersonas++;
            if (cambio == Cambio::Matriculas)
                avisosMatriculas++;
        });

    cout << "Carreras y personas\n";
    s.registrarCarrera("Sistemas");
    s.registrarCarrera("Industrial");
    verificarError<DatosInvalidos>([&] { s.registrarCarrera("Sistemas"); },
                                   "Rechaza una carrera repetida");
    registrar(s, "Estudiante", "Ana Torres", "20231234", "Sistemas");
    registrar(s, "Estudiante", "Luis Quispe", "20232001", "Sistemas");
    registrar(s, "Estudiante", "Maria Huaman", "20262002", "Sistemas");
    registrar(s, "Estudiante", "Rosa Mamani", "20242010", "Industrial");
    registrar(s, "Docente", "Beatriz Salinas", "D-0456", "Sistemas");
    registrar(s, "Administrativo", "Elena Paredes", "A-0789", "Registro");
    verificar(s.getPersonas().size() == 6 && avisosPersonas == 8,
              "La fabrica crea los tres tipos y cada cambio se notifica");
    verificarError<CodigoDuplicado>(
        [&] { registrar(s, "Docente", "Otro", "20231234", "Sistemas"); },
        "Rechaza un codigo repetido");
    verificarError<DatosInvalidos>(
        [&] { registrar(s, "Estudiante", "", "20239999", "Sistemas"); },
        "Rechaza una persona sin nombre");
    verificarError<NoEncontrado>(
        [&] { registrar(s, "Estudiante", "Otro", "20239998", "Medicina"); },
        "Rechaza un estudiante de una carrera no registrada");
    verificarError<DatosInvalidos>(
        [&] { registrar(s, "Visitante", "Otro", "V-1", "Ninguno"); },
        "La fabrica rechaza un tipo no registrado");

    cout << "\nAulas\n";
    s.registrarAula("A-301", 40);
    for (const string aula : { "A-205", "A-206", "L-101", "A-1", "A-999" })
        s.registrarAula(aula, 30);
    s.registrarAula("L-999", 20);
    verificar(s.getAulas().size() == 7, "Registra las aulas del local");
    verificarError<CodigoDuplicado>([&] { s.registrarAula("A-301", 30); },
                                    "Rechaza un aula repetida");
    verificarError<DatosInvalidos>([&] { s.registrarAula("A-500", 0); },
                                   "Rechaza un aula sin capacidad");

    cout << "\nCursos\n";
    s.crearCurso({ "IS-401", "Tecnologia de Objetos", "Sistemas", 4, 6, "A-301",
                   2, h("LU 07-09, MI 07-09"), "D-0456" });
    s.crearCurso({ "IS-402", "Algoritmos", "Sistemas", 4, 6, "A-205", 30,
                   h("LU 08-10"), "" });
    s.crearCurso(
        { "EG-101", "Calculo", "", 4, 1, "A-206", 30, h("MA 07-09"), "" });
    s.crearCurso({ "IS-404", "Taller", "Sistemas", 3, 6, "L-101", 30,
                   h("JU 07-09"), "" });
    verificar(s.getCursos().size() == 4 &&
                  s.buscarCurso("IS-401")->getDocente() ==
                      s.buscarPersona("D-0456") &&
                  s.buscarCurso("EG-101")->esGeneral(),
              "Crea cursos de carrera y generales con su docente");
    verificarError<DatosInvalidos>([&] { h("LU 9-11"); },
                                   "Rechaza un horario mal escrito");
    verificarError<DatosInvalidos>(
        [&]
        {
            Horario horario;
            horario.agregar({ 0, 7, 9 });
            horario.agregar({ 0, 8, 10 });
        },
        "Rechaza un bloque que se superpone con otro del mismo dia");
    verificarError<DatosInvalidos>([&] { h("SA 08-10"); },
                                   "Rechaza clases el sabado");
    verificarError<DatosInvalidos>(
        [&]
        {
            s.crearCurso({ "IS-499", "Mal", "Sistemas", 3, 6, "A-1", 30,
                           Horario(), "" });
        },
        "Rechaza un curso sin horario");
    verificarError<NoEncontrado>(
        [&]
        {
            s.crearCurso({ "IS-499", "Mal", "Medicina", 3, 6, "A-1", 30,
                           h("VI 09-11"), "" });
        },
        "Rechaza un curso de una carrera no registrada");
    verificarError<NoEncontrado>(
        [&]
        {
            s.crearCurso({ "IS-499", "Mal", "Sistemas", 3, 6, "A-1", 30,
                           h("VI 09-11"), "20231234" });
        },
        "Rechaza un docente que no es docente");
    verificarError<AulaOcupada>(
        [&]
        {
            s.crearCurso({ "IS-499", "Otro", "Sistemas", 3, 6, "A-301", 30,
                           h("MI 08-10"), "" });
        },
        "Rechaza usar el aula A-301 a la misma hora");
    verificarError<DocenteOcupado>(
        [&]
        {
            s.crearCurso({ "IS-499", "Otro", "Sistemas", 3, 6, "A-999", 30,
                           h("LU 08-09"), "D-0456" });
        },
        "Rechaza que Beatriz Salinas dicte dos cursos a la vez");
    verificarError<NoEncontrado>(
        [&]
        {
            s.crearCurso({ "IS-499", "Otro", "Sistemas", 3, 6, "B-100", 30,
                           h("VI 09-11"), "" });
        },
        "Rechaza un aula que no esta registrada");
    verificarError<DatosInvalidos>(
        [&]
        {
            s.crearCurso({ "IS-499", "Otro", "Sistemas", 3, 6, "A-205", 31,
                           h("VI 09-11"), "" });
        },
        "Rechaza mas vacantes que asientos tiene el aula");
    verificarError<EnUso>([&] { s.editarAula("A-205", 20); },
                          "No reduce el aula por debajo de las vacantes de "
                          "Algoritmos");
    s.editarAula("A-301", 35);
    verificar(s.buscarCurso("IS-401")->getAula().getCapacidad() == 35,
              "El curso ve la nueva capacidad de su aula");
    verificarError<EnUso>([&] { s.eliminarAula("A-301"); },
                          "No elimina un aula que usa un curso");
    s.eliminarAula("L-999");
    verificar(s.buscarAula("L-999") == nullptr,
              "Elimina un aula que ningun curso usa");

    cout << "\nMatricula\n";
    s.matricular("20231234", "IS-401", "13/11/2026");
    s.matricular("20262002", "IS-401", "13/11/2026");
    verificarError<MatriculaDuplicada>(
        [&] { s.matricular("20231234", "IS-401", "14/11/2026"); },
        "Rechaza una matricula repetida");
    verificarError<CursoSinCupo>(
        [&] { s.matricular("20232001", "IS-401", "14/11/2026"); },
        "Rechaza la tercera matricula en un curso de 2 vacantes");
    verificar(s.buscarCurso("IS-401")->getInscritos() == 2 &&
                  avisosMatriculas == 2,
              "Los rechazos no ocupan cupo ni se notifican");
    verificarError<OtraCarrera>(
        [&] { s.matricular("20242010", "IS-402", "14/11/2026"); },
        "Rechaza un curso de Sistemas para una estudiante de Industrial");
    s.matricular("20242010", "EG-101", "14/11/2026");
    verificar(s.creditosDe(
                  *dynamic_cast<Estudiante*>(s.buscarPersona("20242010"))) == 4,
              "Un curso general admite a cualquier carrera");
    verificarError<CruceHorario>(
        [&] { s.matricular("20231234", "IS-402", "14/11/2026"); },
        "Rechaza un curso que se cruza con LU 07-09");
    s.matricular("20231234", "EG-101", "14/11/2026");
    verificarError<LimiteCreditos>(
        [&] { s.matricular("20231234", "IS-404", "14/11/2026"); },
        "Rechaza pasar el maximo de 10 creditos");

    cout << "\nLista de espera y retiro\n";
    verificar(s.ponerEnEspera("20232001", "IS-401") == 1,
              "Luis Quispe entra primero a la lista de espera");
    verificarError<DatosInvalidos>([&]
                                   { s.ponerEnEspera("20232001", "IS-401"); },
                                   "No entra dos veces a la misma lista");
    verificarError<DatosInvalidos>([&]
                                   { s.ponerEnEspera("20232001", "IS-402"); },
                                   "No espera por un curso con cupos");
    auto promovidos = s.retirar("20262002", "IS-401", "15/11/2026");
    verificar(promovidos.size() == 1 &&
                  promovidos[0]->getCodigo() == "20232001" &&
                  s.buscarCurso("IS-401")->enEspera() == 0,
              "Al retirar a Maria entra Luis desde la lista de espera");
    verificarError<NoEncontrado>(
        [&] { s.retirar("20262002", "IS-401", "15/11/2026"); },
        "No retira una matricula que no existe");

    cout << "\nEdicion y eliminacion de cursos\n";
    DatosCurso objetos = { "IS-401", "Tecnologia de Objetos", "Sistemas", 4,
                           6, "A-301", 2, h("LU 07-09, MI 07-09"), "D-0456" };
    verificar(s.ponerEnEspera("20262002", "IS-401") == 1,
              "Maria entra a la lista de espera del curso lleno");
    objetos.vacantes = 1;
    verificarError<DatosInvalidos>(
        [&] { s.editarCurso(objetos, "16/11/2026"); },
        "No deja menos vacantes que matriculados");
    objetos.vacantes = 2;
    objetos.carrera = "Industrial";
    verificarError<OtraCarrera>(
        [&] { s.editarCurso(objetos, "16/11/2026"); },
        "No pasa a Industrial un curso con matriculados de Sistemas");
    objetos.carrera = "Sistemas";
    verificarError<CruceHorario>(
        [&]
        {
            s.editarCurso({ "EG-101", "Calculo", "", 4, 1, "A-206", 30,
                            h("LU 08-10"), "" },
                          "16/11/2026");
        },
        "No mueve Calculo a una hora que cruza con otro curso de Ana");
    verificarError<AulaOcupada>(
        [&]
        {
            s.editarCurso({ "EG-101", "Calculo", "", 4, 1, "A-205", 30,
                            h("LU 08-10"), "" },
                          "16/11/2026");
        },
        "No mueve Calculo a un aula ocupada a esa hora");
    objetos.nombre = "Tecnologia de Objetos II";
    objetos.vacantes = 3;
    auto entraron = s.editarCurso(objetos, "16/11/2026");
    verificar(entraron.size() == 1 && entraron[0]->getCodigo() == "20262002" &&
                  s.buscarCurso("IS-401")->getNombre() ==
                      "Tecnologia de Objetos II",
              "Al subir las vacantes entra Maria desde la lista de espera");
    s.retirar("20262002", "IS-401", "16/11/2026");
    verificarError<EnUso>([&] { s.eliminarCurso("IS-401"); },
                          "No elimina un curso con matriculados");
    s.eliminarCurso("IS-404");
    verificar(s.buscarCurso("IS-404") == nullptr && s.getCursos().size() == 3,
              "Elimina un curso sin matriculados");

    cout << "\nEdicion y eliminacion de personas\n";
    s.editarPersona("20262002", "Maria Huaman Rojas", "Industrial");
    verificar(s.buscarPersona("20262002")->datoPropio() == "Industrial",
              "Cambia de carrera a quien no lleva cursos de su carrera");
    verificarError<EnUso>(
        [&] { s.editarPersona("20231234", "Ana Torres", "Industrial"); },
        "No cambia de carrera a quien lleva cursos de Sistemas");
    verificarError<EnUso>([&] { s.eliminarPersona("20231234"); },
                          "No elimina a un estudiante con matriculas");
    verificarError<EnUso>([&] { s.eliminarPersona("D-0456"); },
                          "No elimina a un docente con cursos a cargo");
    s.eliminarPersona("A-0789");
    verificar(s.buscarPersona("A-0789") == nullptr,
              "Elimina a quien no tiene datos asociados");

    cout << "\nArchivos CSV\n";
    SistemaMatricula d;
    agregarReglas(d, 25);
    auto cargar = [&d](const string& nombre)
    {
        ifstream archivo("datos/" + nombre + ".csv");
        try
        {
            RepositorioCsv::cargar(archivo, d);
            return archivo.eof();
        }
        catch (const exception& e)
        {
            cout << e.what() << '\n';
            return false;
        }
    };
    auto enEspera = [&d]
    {
        int total = 0;
        for (const auto& c : d.getCursos())
            total += c->enEspera();
        return total;
    };

    verificar(cargar("01_inicio_semestre") && d.getCursos().size() == 40 &&
                  d.getAulas().size() == 8 && d.getMatriculas().empty(),
              "01 abre el semestre con 40 cursos, 8 aulas y sin matriculas");

    verificar(cargar("02_matricula_en_curso") &&
                  d.getMatriculas().size() == 10 && enEspera() == 2,
              "02 trae 10 matriculas y 2 estudiantes en espera");

    verificar(cargar("03_casos_limite") && d.getMatriculas().size() == 15,
              "03 carga los casos limite con 15 matriculas");
    verificarError<CursoSinCupo>(
        [&] { d.matricular("20242010", "IS-0603", "17/11/2026"); },
        "03: Tecnologias de Objetos esta lleno");
    verificarError<LimiteCreditos>(
        [&] { d.matricular("20232001", "IS-0101", "17/11/2026"); },
        "03: Luis Quispe, con 23 creditos, no puede sumar 4 mas");
    verificarError<CruceHorario>(
        [&] { d.matricular("20231234", "IS-0204", "17/11/2026"); },
        "03: IS-0204 cruza el viernes con Tecnologias de Objetos");
    verificarError<FaltaPrerrequisito>(
        [&] { d.matricular("20262002", "IS-0204", "17/11/2026"); },
        "03: Maria no aprobo IS-0103, prerrequisito de IS-0204");
    d.matricular("20242010", "IS-0404", "17/11/2026");
    verificar(d.getMatriculas().size() == 16,
              "03: Rosa aprobo IS-0303 y se matricula en IS-0404");
    auto promovidosCsv = d.retirar("20231234", "IS-0603", "17/11/2026");
    verificar(promovidosCsv.size() == 1 &&
                  promovidosCsv[0]->getCodigo() == "20222015" &&
                  d.buscarCurso("IS-0603")->enEspera() == 1,
              "03: el retiro de Ana da el cupo al primero de la espera");
    ostringstream primera, segunda;
    RepositorioCsv::guardar(d, primera);
    istringstream copia(primera.str());
    RepositorioCsv::cargar(copia, d);
    RepositorioCsv::guardar(d, segunda);
    verificar(primera.str() == segunda.str() && !d.getMatriculas().empty(),
              "Guardar y volver a cargar da el mismo archivo");
    size_t personasAntes = d.getPersonas().size();
    string aviso;
    try
    {
        ifstream danado("datos/04_archivo_danado.csv");
        RepositorioCsv::cargar(danado, d);
    }
    catch (const DatosInvalidos& e)
    {
        aviso = e.what();
    }
    verificar(aviso.rfind("Línea 7 del archivo", 0) == 0,
              "04 avisa la linea 7, que usa un aula no registrada");
    verificar(d.getPersonas().size() == personasAntes,
              "Un archivo danado no borra los datos actuales");
    istringstream exceso("carrera;Sistemas\n"
                         "persona;Estudiante;1;Ana;Sistemas\n"
                         "aula;A-1;30\n"
                         "aula;A-2;30\n"
                         "curso;IS-1;A;;20;1;A-1;30;LU 07-09;\n"
                         "curso;IS-2;B;;20;1;A-2;30;MA 07-09;\n"
                         "matricula;1;IS-1;13/11/2026\n"
                         "matricula;1;IS-2;13/11/2026\n");
    verificarError<DatosInvalidos>(
        [&] { RepositorioCsv::cargar(exceso, d); },
        "Al cargar un archivo se aplican las mismas reglas copiadas");

    cout << '\n' << superadas << " de " << total << " pruebas superadas\n";
    return superadas == total ? 0 : 1;
}
