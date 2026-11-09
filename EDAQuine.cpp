#include <array>
#include <bitset>
#include <cctype>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

struct Termo {
    string bits;
    set<int> mintermos;
};

using Produto = array<int, 4>;
// -1: variavel ausente; 0: negada; 1: normal.

// Converte a string SOP em uma lista de produtos.
vector<Produto> lerSOP(const string& entrada) {
    string s;

    for (unsigned char c : entrada) {
        if (!isspace(c)) {
            s += static_cast<char>(toupper(c));
        }
    }

    if (s.empty()) {
        throw invalid_argument("A expressao esta vazia.");
    }

    vector<Produto> produtos;
    size_t i = 0;

    while (i < s.size()) {
        Produto produto = {-1, -1, -1, -1};
        bool contraditorio = false;

        while (true) {
            bool negada = false;

            if (i < s.size() && s[i] == '!') {
                negada = true;
                ++i;
            }

            if (i >= s.size() || s[i] < 'A' || s[i] > 'D') {
                throw invalid_argument("Esperada uma variavel entre A e D.");
            }

            int variavel = s[i++] - 'A';
            int valor = negada ? 0 : 1;

            if (produto[variavel] != -1 &&
                produto[variavel] != valor) {
                contraditorio = true;
            }

            produto[variavel] = valor;

            if (i == s.size() || s[i] == '+') {
                break;
            }

            if (s[i] != '*') {
                throw invalid_argument("Esperado '*' ou '+'.");
            }

            ++i;
        }

        // Um produto como A*!A e sempre falso.
        if (!contraditorio) {
            produtos.push_back(produto);
        }

        if (i < s.size()) {
            ++i; // Consome '+'.

            if (i == s.size()) {
                throw invalid_argument("Falta um produto depois de '+'.");
            }
        }
    }

    return produtos;
}

// Avalia a SOP nas 16 combinacoes de A, B, C e D.
vector<Termo> listarMintermos(const vector<Produto>& produtos) {
    vector<Termo> termos;

    for (int m = 0; m < 16; ++m) {
        bool funcaoVerdadeira = false;

        for (const Produto& produto : produtos) {
            bool produtoVerdadeiro = true;

            for (int v = 0; v < 4; ++v) {
                int bit = (m >> (3 - v)) & 1;

                if (produto[v] != -1 && produto[v] != bit) {
                    produtoVerdadeiro = false;
                    break;
                }
            }

            if (produtoVerdadeiro) {
                funcaoVerdadeira = true;
                break;
            }
        }

        if (funcaoVerdadeira) {
            termos.push_back({bitset<4>(m).to_string(), {m}});
        }
    }

    return termos;
}

int contarUns(const string& bits) {
    int quantidade = 0;

    for (char bit : bits) {
        if (bit == '1') {
            ++quantidade;
        }
    }

    return quantidade;
}

void imprimirMintermos(const set<int>& mintermos) {
    cout << "{";
    bool primeiro = true;

    for (int m : mintermos) {
        if (!primeiro) {
            cout << ", ";
        }

        cout << m;
        primeiro = false;
    }

    cout << "}";
}

// Combina termos com os mesmos tracos e apenas um bit diferente.
bool combinar(const Termo& a, const Termo& b, Termo& resultado) {
    int diferencas = 0;
    string bits = a.bits;

    for (int i = 0; i < 4; ++i) {
        if (a.bits[i] == b.bits[i]) {
            continue;
        }

        if (a.bits[i] == '-' || b.bits[i] == '-') {
            return false;
        }

        ++diferencas;
        bits[i] = '-';
    }

    if (diferencas != 1) {
        return false;
    }

    resultado = {bits, a.mintermos};
    resultado.mintermos.insert(b.mintermos.begin(), b.mintermos.end());

    return true;
}

// Um mesmo termo pode ser gerado por diferentes combinacoes.
void adicionarSemRepetir(vector<Termo>& termos, const Termo& novo) {
    for (Termo& termo : termos) {
        if (termo.bits == novo.bits) {
            termo.mintermos.insert(
                novo.mintermos.begin(), novo.mintermos.end()
            );
            return;
        }
    }

    termos.push_back(novo);
}

vector<Termo> executarEtapa1(vector<Termo> atuais) {
    vector<Termo> primos;
    int rodada = 1;

    while (!atuais.empty()) {
        // Cada grupo armazena indices do vetor "atuais".
        array<vector<size_t>, 5> grupos;

        for (size_t i = 0; i < atuais.size(); ++i) {
            grupos[contarUns(atuais[i].bits)].push_back(i);
        }

        cout << "\n=== Rodada " << rodada++ << " ===\n";

        for (int g = 0; g <= 4; ++g) {
            cout << "Grupo " << g << ":\n";

            for (size_t indice : grupos[g]) {
                cout << "  " << atuais[indice].bits << "  ";
                imprimirMintermos(atuais[indice].mintermos);
                cout << '\n';
            }
        }

        vector<bool> combinados(atuais.size(), false);
        vector<Termo> proximos;

        cout << "\nTermo 1 | Termo 2 | Resultado | Mintermos\n";

        for (int g = 0; g < 4; ++g) {
            for (size_t i : grupos[g]) {
                for (size_t j : grupos[g + 1]) {
                    Termo resultado;

                    if (combinar(atuais[i], atuais[j], resultado)) {
                        combinados[i] = true;
                        combinados[j] = true;

                        cout << atuais[i].bits << "    | "
                             << atuais[j].bits << "    | "
                             << resultado.bits << "      | ";

                        imprimirMintermos(resultado.mintermos);
                        cout << '\n';

                        adicionarSemRepetir(proximos, resultado);
                    }
                }
            }
        }

        // Termos que nao puderam ser combinados sao primos.
        for (size_t i = 0; i < atuais.size(); ++i) {
            if (!combinados[i]) {
                adicionarSemRepetir(primos, atuais[i]);
            }
        }

        if (proximos.empty()) {
            cout << "Nenhuma nova combinacao.\n";
        }

        atuais = proximos;
    }

    return primos;
}

int main() {
    cout << "Digite a expressao SOP usando !, * e +:\n";

    string entrada;
    getline(cin, entrada);

    try {
        vector<Produto> produtos = lerSOP(entrada);
        vector<Termo> mintermos = listarMintermos(produtos);

        cout << "\nMintermos da funcao:\n";

        for (const Termo& termo : mintermos) {
            cout << "m" << *termo.mintermos.begin()
                 << " = " << termo.bits << '\n';
        }

        if (mintermos.empty()) {
            cout << "A funcao e sempre 0.\n";
            return 0;
        }

        vector<Termo> primos = executarEtapa1(mintermos);

        cout << "\nImplicantes primos:\n";

        for (const Termo& termo : primos) {
            cout << termo.bits << "  ";
            imprimirMintermos(termo.mintermos);
            cout << '\n';
        }
    } catch (const exception& erro) {
        cerr << "Erro: " << erro.what() << '\n';
        return 1;
    }

    return 0;
}