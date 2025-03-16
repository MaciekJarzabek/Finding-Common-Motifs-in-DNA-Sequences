#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <map>
#include <utility>
#include <cstdlib>
#include <iterator>

using namespace std;

struct KwasNukleinowy {
    int sekwencja;
    int pozycja;
    int jakosc;
    char nukleotyd;
};

struct Wierzcholek {
    string podciag;
    int w_sekwencja = 0;
    int w_pozycja = 0;
};

int prog, dlugosc;
vector<KwasNukleinowy> dane_kwasow;
map<pair<string, pair<int, int>>, vector<pair<string, pair<int, int>>>> lista_sasiedztwa;

void czytaj_pliki(string plikFasta, string plikQual) {
    fstream plikF(plikFasta, ios::in);
    fstream plikQ(plikQual, ios::in);
    string linia;
    int num_sekw = 0;
    int num_poz = 0;
    int a = 0;
    vector<int> jakosci_sekw;

    if (plikQ.is_open()) {
        while (getline(plikQ, linia)) {
            if (linia[0] != '>') {
                istringstream ss(linia);
                int n;
                while (ss >> n) {
                    jakosci_sekw.push_back(n);
                    ss.ignore();
                }
            }
        }
    }
    if (plikF.is_open()) {
        while (getline(plikF, linia)) {
            if (linia[0] != '>') {
                for (auto i : linia) {
                    num_poz++;
                    KwasNukleinowy kwas;
                    kwas.nukleotyd = i;
                    kwas.pozycja = num_poz;
                    kwas.sekwencja = num_sekw;
                    kwas.jakosc = jakosci_sekw[a++];
                    dane_kwasow.push_back(kwas);
                }
            }
            else {
                num_poz = 0;
                num_sekw++;
            }
        }
    }
    plikF.close();
    plikQ.close();
    dane_kwasow.erase(
        remove_if(dane_kwasow.begin(), dane_kwasow.end(),
            [](const KwasNukleinowy& k) { return k.jakosc < prog; }),
        dane_kwasow.end());
}

void utworz_graf() {
    vector<Wierzcholek> dane_wierzcholkow;
    vector<int> pozycje;
    string sekwencja;
    int num_sekw = 1;
    for (auto kwas : dane_kwasow) {
        if (kwas.sekwencja == num_sekw)
            sekwencja += kwas.nukleotyd;
        if (kwas.sekwencja != num_sekw) {
            for (int i = 0; i <= sekwencja.size() - dlugosc; i++) {
                Wierzcholek wierzcholek;
                wierzcholek.podciag = sekwencja.substr(i, dlugosc);
                wierzcholek.w_sekwencja = num_sekw;
                wierzcholek.w_pozycja = pozycje.front();
                pozycje.erase(pozycje.begin());
                dane_wierzcholkow.push_back(wierzcholek);
            }
            sekwencja.clear();
            pozycje.clear();
            num_sekw = kwas.sekwencja;
            sekwencja += kwas.nukleotyd;
        }
        pozycje.push_back(kwas.pozycja);
    }
    for (int i = 0; i <= sekwencja.size() - dlugosc; i++) {
        Wierzcholek wierzcholek;
        wierzcholek.podciag = sekwencja.substr(i, dlugosc);
        wierzcholek.w_sekwencja = num_sekw;
        wierzcholek.w_pozycja = pozycje.front();
        pozycje.erase(pozycje.begin());
        dane_wierzcholkow.push_back(wierzcholek);
    }
    for (size_t i = 0; i < dane_wierzcholkow.size(); ++i) {
        for (size_t j = i + 1; j < dane_wierzcholkow.size(); ++j) {
            if (dane_wierzcholkow[i].podciag == dane_wierzcholkow[j].podciag &&
                dane_wierzcholkow[i].w_sekwencja != dane_wierzcholkow[j].w_sekwencja) {
                if (abs(dane_wierzcholkow[i].w_pozycja - dane_wierzcholkow[j].w_pozycja) <= dlugosc * 10) {
                    pair<int, int> a1(dane_wierzcholkow[i].w_sekwencja, dane_wierzcholkow[i].w_pozycja);
                    pair<string, pair<int, int>> a2(dane_wierzcholkow[i].podciag, a1);
                    pair<int, int> b1(dane_wierzcholkow[j].w_sekwencja, dane_wierzcholkow[j].w_pozycja);
                    pair<string, pair<int, int>> b2(dane_wierzcholkow[j].podciag, b1);
                    lista_sasiedztwa[a2].push_back(b2);
                    lista_sasiedztwa[b2].push_back(a2);
                }
            }
        }
    }
}

vector<pair<string, pair<int, int>>> znajdz_kliki() {
    vector<pair<string, pair<int, int>>> klika;
    for (auto i = lista_sasiedztwa.begin(); i != lista_sasiedztwa.end(); i++) {
        klika.clear();
        klika.push_back(i->first);
        for (auto const& j : lista_sasiedztwa) {
            if (find(klika.begin(), klika.end(), j.first) != klika.end())
                continue;
            bool w_klice = true;
            for (auto const& u : klika) {
                if (find(j.second.begin(), j.second.end(), u) != j.second.end())
                    continue;
                else {
                    w_klice = false;
                    break;
                }
            }
            if (w_klice) {
                klika.push_back(j.first);
                if (klika.size() == 5)
                    return klika;
            }
        }
    }
    return vector<pair<string, pair<int, int>>>();
}

int main() {
    cout << "Podaj dlugosc podciagow (4-9):" << endl;
    cin >> dlugosc;
    cout << "Podaj prog minimalnej jakosci:" << endl;
    cin >> prog;
    czytaj_pliki("wejscie1.txt", "wejscie2.txt");
    utworz_graf();
    vector<pair<string, pair<int, int>>> wynik = znajdz_kliki();
    if (wynik.empty()) {
        cout << "Nie znaleziono motywu na 5 sekwencjach" << endl;
    }
    else {
        cout << "Znaleziono motyw: " << wynik[0].first << endl;
        for (const auto& elem : wynik) {
            cout << "Sekwencja: " << elem.second.first << " Pozycja w sekwencji: " << elem.second.second << endl;
        }
    }
    return 0;
}