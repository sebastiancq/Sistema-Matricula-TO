#include "modelo/RepositorioCsv.h"
#include "modelo/Excepciones.h"
#include "modelo/PersonaFactory.h"
#include <sstream>

using namespace std;

static vector<string> separar(const string& linea)
{
    vector<string> campos;
    stringstream entrada(linea);
    string campo;
    while (getline(entrada, campo, ';'))
        campos.push_back(campo);
    if (!linea.empty() && linea.back() == ';')
        campos.push_back("");
    return campos;
}

static vector<string> palabras(const string& texto)
{
    vector<string> lista;
    stringstream entrada(texto);
    string palabra;
    while (entrada >> palabra)
        lista.push_back(palabra);
    return lista;
}

static string unir(const vector<string>& lista)
{
    string texto;
    for (size_t i = 0; i < lista.size(); i++)
        texto += (i == 0 ? "" : " ") + lista[i];
    return texto;
}

static int entero(const string& texto)
{
    try
    {
        return stoi(texto);
    }
    catch (const exception&)
    {
        throw DatosInvalidos("Se esperaba un número en lugar de «" + texto +
                             "».");
    }
}

void RepositorioCsv::guardar(const SistemaMatricula& sistema, ostream& salida)
{
    salida << "# Sistema de matrícula\n";
    for (const string& carrera : sistema.getCarreras())
        salida << "carrera;" << carrera << '\n';
    for (const auto& p : sistema.getPersonas())
        salida << "persona;" << p->tipo() << ';' << p->getCodigo() << ';'
               << p->getNombre() << ';' << p->datoPropio() << '\n';
    for (const auto& p : sistema.getPersonas())
    {
        auto* e = dynamic_cast<const Estudiante*>(p.get());
        if (e && !e->getAprobados().empty())
            salida << "aprobados;" << e->getCodigo() << ';'
                   << unir(e->getAprobados()) << '\n';
    }
    for (const auto& a : sistema.getAulas())
        salida << "aula;" << a->getCodigo() << ';' << a->getCapacidad() << '\n';
    for (const auto& c : sistema.getCursos())
        salida << "curso;" << c->getCodigo() << ';' << c->getNombre() << ';'
               << c->getCarrera() << ';' << c->getCreditos() << ';'
               << c->getSemestre() << ';' << c->getAula().getCodigo() << ';'
               << c->getVacantes() << ';' << c->getHorario() << ';'
               << (c->getDocente() ? c->getDocente()->getCodigo() : "") << ';'
               << unir(c->getPrerrequisitos()) << '\n';
    for (const Matricula& m : sistema.getMatriculas())
        salida << "matricula;" << m.estudiante->getCodigo() << ';'
               << m.curso->getCodigo() << ';' << m.fecha << '\n';
    for (const auto& c : sistema.getCursos())
        for (const Estudiante* e : c->estudiantesEnEspera())
            salida << "espera;" << c->getCodigo() << ';' << e->getCodigo()
                   << '\n';
}

// Cada linea se aplica con las mismas operaciones del sistema, asi los datos
// del archivo pasan por las mismas reglas que los del formulario. Se cargan en
// una copia vacia y solo al final reemplazan a los actuales.
void RepositorioCsv::cargar(istream& entrada, SistemaMatricula& sistema)
{
    SistemaMatricula nuevo = sistema.copiaVacia();
    string linea;
    int numeroLinea = 0;
    while (getline(entrada, linea))
    {
        numeroLinea++;
        if (!linea.empty() && linea.back() == '\r')
            linea.pop_back();
        if (linea.empty() || linea[0] == '#')
            continue;

        vector<string> c = separar(linea);
        try
        {
            if (c[0] == "carrera" && c.size() == 2)
                nuevo.registrarCarrera(c[1]);
            else if (c[0] == "persona" && c.size() == 5)
                nuevo.registrarPersona(
                    PersonaFactory::crearPersona(c[1], c[3], c[2], c[4]));
            else if (c[0] == "aula" && c.size() == 3)
                nuevo.registrarAula(c[1], entero(c[2]));
            else if (c[0] == "aprobados" && c.size() == 3)
            {
                auto* e = dynamic_cast<Estudiante*>(nuevo.buscarPersona(c[1]));
                if (e == nullptr)
                    throw NoEncontrado("No hay un estudiante con el código " +
                                       c[1] + ".");
                for (const string& curso : palabras(c[2]))
                    e->agregarAprobado(curso);
            }
            else if (c[0] == "curso" && (c.size() == 10 || c.size() == 11))
                nuevo.crearCurso(
                    { c[1], c[2], c[3], entero(c[4]), entero(c[5]), c[6],
                      entero(c[7]), Horario::desdeTexto(c[8]), c[9],
                      c.size() == 11 ? palabras(c[10]) : vector<string>() });
            else if (c[0] == "matricula" && c.size() == 4)
                nuevo.matricular(c[1], c[2], c[3]);
            else if (c[0] == "espera" && c.size() == 3)
                nuevo.ponerEnEspera(c[2], c[1]);
            else
                throw DatosInvalidos("El registro " + c[0] +
                                     " no es válido; revise el tipo y la "
                                     "cantidad de campos.");
        }
        catch (const exception& e)
        {
            throw DatosInvalidos("Línea " + to_string(numeroLinea) +
                                 " del archivo: " + e.what());
        }
    }
    sistema.reemplazarDatos(nuevo);
}
