#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <cstdlib>

using namespace std;

class Configuracion {
private:
    int numeroJugadores;
    string* palos;
    int cantidadPalos;
    int valorGanar;

    string limpiarTexto(const string& str) const {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (string::npos == first) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

    void liberarPalos() {
        if (palos != nullptr) {
            delete[] palos;
            palos = nullptr;
        }
        cantidadPalos = 0;
    }

    void procesarPalos(string linea) {
        liberarPalos();
        size_t posY = linea.find(" y ");
        while (posY != string::npos) {
            linea.replace(posY, 3, ",");
            posY = linea.find(" y ", posY + 1);
        }

        stringstream ssContar(linea);
        string item;
        int conteo = 0;
        while (getline(ssContar, item, ',')) {
            if (!limpiarTexto(item).empty()) {
                conteo++;
            }
        }

        if (conteo > 0) {
            palos = new string[conteo];
            stringstream ssExtraer(linea);
            int idx = 0;
            while (getline(ssExtraer, item, ',')) {
                string limpio = limpiarTexto(item);
                if (!limpio.empty()) {
                    palos[idx++] = limpio;
                }
            }
            cantidadPalos = conteo;
        }
    }

public:
    Configuracion() : numeroJugadores(3), palos(nullptr), cantidadPalos(0), valorGanar(21) {}

    ~Configuracion() {
        liberarPalos();
    }

    bool cargarDesdeArchivo(const string& nombreArchivo) {
        ifstream archivo(nombreArchivo.c_str());
        if (!archivo.is_open()) {
            return false;
        }
        string linea1, linea2, linea3;
        if (!getline(archivo, linea1)) return false;
        if (!getline(archivo, linea2)) return false;
        if (!getline(archivo, linea3)) return false;

        try {
            numeroJugadores = stoi(limpiarTexto(linea1));
            procesarPalos(linea2);
            valorGanar = stoi(limpiarTexto(linea3));
        } catch (...) {
            archivo.close();
            return false;
        }
        archivo.close();
        return (numeroJugadores > 0 && cantidadPalos > 0 && valorGanar > 0);
    }

    void guardarEnArchivo(const string& nombreArchivo, int numJug, const string& strPalos, int valGanar) {
        ofstream archivo(nombreArchivo.c_str());
        if (archivo.is_open()) {
            archivo << numJug << "\n";
            archivo << strPalos << "\n";
            archivo << valGanar << "\n";
            archivo.close();
        }
    }

    int getNumeroJugadores() const { return numeroJugadores; }
    string* getPalos() const { return palos; }
    int getCantidadPalos() const { return cantidadPalos; }
    int getValorGanar() const { return valorGanar; }

    void mostrarConfiguracion() const {
        cout << "==========================================" << endl;
        cout << "       CONFIGURACION ACTUAL (setup.txt)   " << endl;
        cout << "==========================================" << endl;
        cout << "Numero de jugadores : " << numeroJugadores << endl;
        cout << "Palos de las cartas : ";
        for (int i = 0; i < cantidadPalos; ++i) {
            cout << palos[i] << (i < cantidadPalos - 1 ? ", " : "");
        }
        cout << endl;
        cout << "Valor para ganar    : " << valorGanar << endl;
        cout << "==========================================" << endl;
    }
};

class Carta {
private:
    int valor;
    string nombre;
    string palo;

public:
    Carta() : valor(0), nombre("Nula"), palo("Ninguno") {}
    Carta(int v, string n, string p) : valor(v), nombre(n), palo(p) {}

    int getValor() const { return valor; }
    string getNombre() const { return nombre; }
    string getPalo() const { return palo; }

    string toString() const {
        return nombre + " de " + palo + " (" + to_string(valor) + " pts)";
    }
};

class Mazo {
private:
    Carta* cartas;
    int totalCartas;
    int indiceActual;

    void liberarMazo() {
        if (cartas != nullptr) {
            delete[] cartas;
            cartas = nullptr;
        }
        totalCartas = 0;
        indiceActual = 0;
    }

public:
    Mazo() : cartas(nullptr), totalCartas(0), indiceActual(0) {}

    ~Mazo() {
        liberarMazo();
    }

    void inicializar(string* palos, int cantidadPalos) {
        liberarMazo();
        if (cantidadPalos <= 0) return;

        totalCartas = cantidadPalos * 13;
        cartas = new Carta[totalCartas];
        indiceActual = 0;

        string nombres[] = {"As", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
        int valores[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10};

        int idx = 0;
        for (int i = 0; i < cantidadPalos; ++i) {
            for (int j = 0; j < 13; ++j) {
                cartas[idx++] = Carta(valores[j], nombres[j], palos[i]);
            }
        }
    }

    void barajar() {
        for (int i = 0; i < totalCartas; ++i) {
            int r = i + rand() % (totalCartas - i);
            Carta temp = cartas[i];
            cartas[i] = cartas[r];
            cartas[r] = temp;
        }
        indiceActual = 0;
    }

    bool quedanCartas() const {
        return indiceActual < totalCartas;
    }

    Carta repartir() {
        if (quedanCartas()) {
            return cartas[indiceActual++];
        }
        return Carta(0, "Nula", "Ninguno");
    }
};

class Jugador {
private:
    string nombre;
    Carta* mano;
    int cantidadCartas;
    int capacidadMano;
    bool plantado;

    void redimensionarMano(int nuevaCapacidad) {
        Carta* nuevaMano = new Carta[nuevaCapacidad];
        for (int i = 0; i < cantidadCartas; ++i) {
            nuevaMano[i] = mano[i];
        }
        if (mano != nullptr) {
            delete[] mano;
        }
        mano = nuevaMano;
        capacidadMano = nuevaCapacidad;
    }

public:
    Jugador() : nombre("Jugador"), mano(nullptr), cantidadCartas(0), capacidadMano(0), plantado(false) {}

    Jugador(string n) : nombre(n), mano(nullptr), cantidadCartas(0), capacidadMano(0), plantado(false) {}

    Jugador(const Jugador& otro) : nombre(otro.nombre), mano(nullptr), cantidadCartas(otro.cantidadCartas), capacidadMano(otro.capacidadMano), plantado(otro.plantado) {
        if (capacidadMano > 0) {
            mano = new Carta[capacidadMano];
            for (int i = 0; i < cantidadCartas; ++i) {
                mano[i] = otro.mano[i];
            }
        }
    }

    Jugador& operator=(const Jugador& otro) {
        if (this != &otro) {
            if (mano != nullptr) {
                delete[] mano;
            }
            nombre = otro.nombre;
            cantidadCartas = otro.cantidadCartas;
            capacidadMano = otro.capacidadMano;
            plantado = otro.plantado;
            if (capacidadMano > 0) {
                mano = new Carta[capacidadMano];
                for (int i = 0; i < cantidadCartas; ++i) {
                    mano[i] = otro.mano[i];
                }
            } else {
                mano = nullptr;
            }
        }
        return *this;
    }

    ~Jugador() {
        if (mano != nullptr) {
            delete[] mano;
            mano = nullptr;
        }
    }

    void configurarNombre(const string& n) {
        nombre = n;
    }

    void recibirCarta(const Carta& c) {
        if (cantidadCartas == capacidadMano) {
            int nuevaCap = (capacidadMano == 0) ? 4 : capacidadMano * 2;
            redimensionarMano(nuevaCap);
        }
        mano[cantidadCartas++] = c;
    }

    void limpiarMano() {
        if (mano != nullptr) {
            delete[] mano;
            mano = nullptr;
        }
        cantidadCartas = 0;
        capacidadMano = 0;
        plantado = false;
    }

    int calcularPuntaje(int valorMeta) const {
        int suma = 0;
        int ases = 0;
        for (int i = 0; i < cantidadCartas; ++i) {
            suma += mano[i].getValor();
            if (mano[i].getNombre() == "As") {
                ases++;
            }
        }
        while (ases > 0 && suma + 10 <= valorMeta) {
            suma += 10;
            ases--;
        }
        return suma;
    }

    string getNombre() const { return nombre; }
    bool isPlantado() const { return plantado; }
    void plantarse() { plantado = true; }

    void mostrarMano(int valorMeta) const {
        cout << "  " << nombre << " tiene:" << endl;
        for (int i = 0; i < cantidadCartas; ++i) {
            cout << "    - " << mano[i].toString() << endl;
        }
        cout << "    -> Puntaje total: " << calcularPuntaje(valorMeta) << endl;
    }
};

class Juego {
private:
    Configuracion config;
    Mazo mazo;
    Jugador* jugadores;
    int totalJugadores;

    void liberarJugadores() {
        if (jugadores != nullptr) {
            delete[] jugadores;
            jugadores = nullptr;
        }
        totalJugadores = 0;
    }

public:
    Juego() : jugadores(nullptr), totalJugadores(0) {}

    ~Juego() {
        liberarJugadores();
    }

    bool inicializar(const string& archivoConfig) {
        if (!config.cargarDesdeArchivo(archivoConfig)) {
            return false;
        }
        mazo.inicializar(config.getPalos(), config.getCantidadPalos());
        mazo.barajar();

        liberarJugadores();
        totalJugadores = config.getNumeroJugadores();
        jugadores = new Jugador[totalJugadores];
        for (int i = 0; i < totalJugadores; ++i) {
            jugadores[i].configurarNombre("Jugador " + to_string(i + 1));
        }
        return true;
    }

    void configurarPrueba(const string& archivoConfig, int opcionPrueba) {
        if (opcionPrueba == 1) {
            config.guardarEnArchivo(archivoConfig, 3, "Negros, Rojos, Verdes y Azules", 21);
        } else if (opcionPrueba == 2) {
            config.guardarEnArchivo(archivoConfig, 4, "Oros, Copas, Espadas, Bastos", 22);
        } else if (opcionPrueba == 3) {
            config.guardarEnArchivo(archivoConfig, 5, "Hearts, Diamonds, Clubs, Spades", 20);
        }
    }

    void mostrarDatosConfiguracion() const {
        config.mostrarConfiguracion();
    }

    void jugarPartida(bool modoAutomatico) {
        int meta = config.getValorGanar();
        cout << "\n--- INICIANDO PARTIDA (META: " << meta << " PUNTOS) ---\n" << endl;

        for (int i = 0; i < totalJugadores; ++i) {
            jugadores[i].limpiarMano();
            jugadores[i].recibirCarta(mazo.repartir());
            jugadores[i].recibirCarta(mazo.repartir());
        }

        for (int i = 0; i < totalJugadores; ++i) {
            cout << "\n>>> Turno de " << jugadores[i].getNombre() << " <<<" << endl;
            while (!jugadores[i].isPlantado() && jugadores[i].calcularPuntaje(meta) < meta && mazo.quedanCartas()) {
                jugadores[i].mostrarMano(meta);
                int puntajeActual = jugadores[i].calcularPuntaje(meta);

                if (modoAutomatico) {
                    if (puntajeActual <= meta - 4) {
                        cout << "  [Auto] " << jugadores[i].getNombre() << " pide una carta..." << endl;
                        jugadores[i].recibirCarta(mazo.repartir());
                    } else {
                        cout << "  [Auto] " << jugadores[i].getNombre() << " decide plantarse." << endl;
                        jugadores[i].plantarse();
                    }
                } else {
                    char decision;
                    cout << "  Desea pedir otra carta? (s/n): ";
                    cin >> decision;
                    if (decision == 's' || decision == 'S') {
                        jugadores[i].recibirCarta(mazo.repartir());
                    } else {
                        jugadores[i].plantarse();
                    }
                }
            }
            jugadores[i].mostrarMano(meta);
            int puntajeFinal = jugadores[i].calcularPuntaje(meta);
            if (puntajeFinal == meta) {
                cout << "  *** " << jugadores[i].getNombre() << " ha alcanzado exactamente " << meta << "! ***" << endl;
            } else if (puntajeFinal > meta) {
                cout << "  xxx " << jugadores[i].getNombre() << " se ha pasado de " << meta << " (Eliminado). xxx" << endl;
            }
        }

        determinarGanadores();
    }

    void determinarGanadores() const {
        int meta = config.getValorGanar();
        int mejorPuntaje = -1;
        string* ganadores = new string[totalJugadores];
        int cantidadGanadores = 0;

        cout << "\n==========================================" << endl;
        cout << "            RESULTADOS FINALES            " << endl;
        cout << "==========================================" << endl;

        for (int i = 0; i < totalJugadores; ++i) {
            int p = jugadores[i].calcularPuntaje(meta);
            cout << jugadores[i].getNombre() << " -> " << p << " puntos ";
            if (p > meta) {
                cout << "(Se paso)" << endl;
            } else {
                cout << "(Valido)" << endl;
                if (p > mejorPuntaje) {
                    mejorPuntaje = p;
                    cantidadGanadores = 0;
                    ganadores[cantidadGanadores++] = jugadores[i].getNombre();
                } else if (p == mejorPuntaje) {
                    ganadores[cantidadGanadores++] = jugadores[i].getNombre();
                }
            }
        }

        cout << "------------------------------------------" << endl;
        if (cantidadGanadores == 0) {
            cout << "Resultado: Todos los jugadores se pasaron de " << meta << ". No hay ganador." << endl;
        } else if (cantidadGanadores == 1) {
            cout << "GANADOR: " << ganadores[0] << " con " << mejorPuntaje << " puntos!" << endl;
        } else {
            cout << "EMPATE entre: ";
            for (int i = 0; i < cantidadGanadores; ++i) {
                cout << ganadores[i] << (i < cantidadGanadores - 1 ? ", " : "");
            }
            cout << " con " << mejorPuntaje << " puntos!" << endl;
        }
        cout << "==========================================\n" << endl;

        delete[] ganadores;
    }
};

int main() {
    srand(static_cast<unsigned int>(time(0)));
    string archivoConfig = "setup.txt";
    Juego juego;

    if (!juego.inicializar(archivoConfig)) {
        juego.configurarPrueba(archivoConfig, 1);
        juego.inicializar(archivoConfig);
    }

    int opcion = 0;
    do {
        juego.mostrarDatosConfiguracion();
        cout << "1. Simular partida automatica con setup.txt actual" << endl;
        cout << "2. Jugar partida manual con setup.txt actual" << endl;
        cout << "3. Cargar Prueba 1 en setup.txt (3 Jugadores | Negros, Rojos, Verdes y Azules | Meta: 21)" << endl;
        cout << "4. Cargar Prueba 2 en setup.txt (4 Jugadores | Oros, Copas, Espadas, Bastos | Meta: 22)" << endl;
        cout << "5. Cargar Prueba 3 en setup.txt (5 Jugadores | Hearts, Diamonds, Clubs, Spades | Meta: 20)" << endl;
        cout << "6. Recargar archivo setup.txt manualmente" << endl;
        cout << "0. Salir" << endl;
        cout << "Seleccione una opcion: ";
        if (!(cin >> opcion)) {
            cin.clear();
            cin.ignore(10000, '\n');
            opcion = -1;
        }

        if (opcion == 1) {
            juego.inicializar(archivoConfig);
            juego.jugarPartida(true);
        } else if (opcion == 2) {
            juego.inicializar(archivoConfig);
            juego.jugarPartida(false);
        } else if (opcion >= 3 && opcion <= 5) {
            juego.configurarPrueba(archivoConfig, opcion - 2);
            juego.inicializar(archivoConfig);
            cout << "\n[Archivo setup.txt actualizado con los datos de la Prueba " << (opcion - 2) << "]\n" << endl;
        } else if (opcion == 6) {
            if (juego.inicializar(archivoConfig)) {
                cout << "\n[Archivo setup.txt recargado correctamente]\n" << endl;
            } else {
                cout << "\n[Error al leer setup.txt. Verifique el formato]\n" << endl;
            }
        }
    } while (opcion != 0);

    return 0;
}
