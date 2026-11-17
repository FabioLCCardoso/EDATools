#include <array>
#include <bitset>
#include <cctype>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include <functional>
#include <iomanip>
#include <limits>
#include <fstream>
#include <sstream>

using namespace std;

struct Termo {
    string bits;
    set<int> mintermos;
};

using Produto = array<int, 4>;
// -1: variavel ausente; 0: negada; 1: normal.



// Converte a string SOP em uma lista de produtos.
vector<Produto> lerSOPSemParenteses(const string& entrada) {
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

vector<Produto> lerSOP(const string& entrada) {
    string compacta;

    for (unsigned char c : entrada) {
        if (!isspace(c)) {
            compacta += static_cast<char>(toupper(c));
        }
    }

    if (compacta.empty()) {
        throw invalid_argument("A expressao esta vazia.");
    }

    string normalizada;
    size_t inicio = 0;

    while (true) {
        size_t fim = compacta.find('+', inicio);

        string produto = compacta.substr(
            inicio,
            fim == string::npos ? string::npos : fim - inicio
        );

        if (produto.empty()) {
            throw invalid_argument("Produto vazio na expressao.");
        }

        // Aceita um par de parenteses envolvendo o produto inteiro.
        if (produto.front() == '(' && produto.back() == ')') {
            produto = produto.substr(1, produto.size() - 2);
        }

        if (produto.empty()) {
            throw invalid_argument("Produto vazio entre parenteses.");
        }

        // Parenteses restantes indicam formato nao suportado
        // ou parenteses desbalanceados.
        if (produto.find_first_of("()") != string::npos) {
            throw invalid_argument(
                "Use parenteses apenas ao redor de cada produto SOP."
            );
        }

        if (!normalizada.empty()) {
            normalizada += '+';
        }

        normalizada += produto;

        if (fim == string::npos) {
            break;
        }

        inicio = fim + 1;
    }

    return lerSOPSemParenteses(normalizada);
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

struct ResultadoCobertura {
    vector<size_t> indices;
    int literais = 0;
};

// Cada caractere diferente de '-' corresponde a um literal.
int contarLiterais(const Termo& termo) {
    int total = 0;

    for (char bit : termo.bits) {
        if (bit != '-') {
            ++total;
        }
    }

    return total;
}

// Converte um padrao binario em um produto SOP.
// Exemplo: "0-01" -> "!A*!C*D".
string termoParaSOP(const Termo& termo) {
    const string variaveis = "ABCD";
    string produto;

    for (size_t i = 0; i < 4; ++i) {
        if (termo.bits[i] == '-') {
            continue;
        }

        if (!produto.empty()) {
            produto += "*";
        }

        if (termo.bits[i] == '0') {
            produto += "!";
        }

        produto += variaveis[i];
    }

    // "----" representa a funcao constante 1.
    return produto.empty() ? "1" : produto;
}

void imprimirTabelaCobertura(
    const vector<Termo>& primos,
    const vector<Termo>& mintermos
) {
    cout << "\n=== Tabela de cobertura ===\n";

    cout << left
         << setw(8) << "Primo"
         << setw(8) << "Bits";

    for (const Termo& termo : mintermos) {
        int m = *termo.mintermos.begin();
        cout << setw(6) << ("m" + to_string(m));
    }

    cout << '\n';

    for (size_t i = 0; i < primos.size(); ++i) {
        cout << setw(8) << ("P" + to_string(i))
             << setw(8) << primos[i].bits;

        for (const Termo& termo : mintermos) {
            int m = *termo.mintermos.begin();

            bool cobre = primos[i].mintermos.count(m) != 0;
            cout << setw(6) << (cobre ? "X" : ".");
        }

        cout << '\n';
    }
}

ResultadoCobertura selecionarCoberturaMinima(
    const vector<Termo>& primos,
    const vector<Termo>& mintermos
) {
    /*
       Uma mascara usa o bit m para indicar o mintermo m.

       Exemplo:
       cobertura de {0, 1} = bits 0 e 1 ligados.

       Como existem apenas 16 mintermos, usamos
       um unsigned int como conjunto de cobertura.
    */

    unsigned int alvo = 0u;

    for (const Termo& termo : mintermos) {
        int m = *termo.mintermos.begin();
        alvo |= (1u << m);
    }

    vector<unsigned int> mascaras(primos.size(), 0u);
    vector<int> custos(primos.size(), 0);

    for (size_t i = 0; i < primos.size(); ++i) {
        for (int m : primos[i].mintermos) {
            mascaras[i] |= (1u << m);
        }

        custos[i] = contarLiterais(primos[i]);
    }

    /*
       restantes[i] representa tudo que pode ser coberto
       pelos implicantes de indice i em diante.
    */
    vector<unsigned int> restantes(primos.size() + 1, 0u);

    for (size_t i = primos.size(); i > 0; --i) {
        restantes[i - 1] = restantes[i] | mascaras[i - 1];
    }

    ResultadoCobertura melhor;
    size_t melhorQuantidade = numeric_limits<size_t>::max();
    int melhorLiterais = numeric_limits<int>::max();

    vector<size_t> escolhidos;

    function<void(size_t, unsigned int, int)> buscar;

    buscar = [&](size_t indice,
                 unsigned int cobertos,
                 int literais) {

        // Encontramos uma cobertura completa.
        if ((cobertos & alvo) == alvo) {
            bool menosTermos =
                escolhidos.size() < melhorQuantidade;

            bool mesmosTermosMenosLiterais =
                escolhidos.size() == melhorQuantidade &&
                literais < melhorLiterais;

            if (menosTermos || mesmosTermosMenosLiterais) {
                melhorQuantidade = escolhidos.size();
                melhorLiterais = literais;

                melhor.indices = escolhidos;
                melhor.literais = literais;
            }

            return;
        }

        // Nao ha mais implicantes para escolher.
        if (indice == primos.size()) {
            return;
        }

        // Ainda falta cobertura e ja usamos tantos termos
        // quanto a melhor solucao: continuar nao melhora.
        if (escolhidos.size() >= melhorQuantidade) {
            return;
        }

        // Nem usando todos os implicantes restantes
        // seria possivel completar a cobertura.
        if (((cobertos | restantes[indice]) & alvo) != alvo) {
            return;
        }

        unsigned int novaCobertura =
            cobertos | mascaras[indice];

        // Opcao 1: incluir o implicante, se cobrir algo novo.
        if (novaCobertura != cobertos) {
            escolhidos.push_back(indice);

            buscar(
                indice + 1,
                novaCobertura,
                literais + custos[indice]
            );

            escolhidos.pop_back();
        }

        // Opcao 2: nao incluir o implicante.
        buscar(indice + 1, cobertos, literais);
    };

    buscar(0, 0u, 0);

    if (melhorQuantidade == numeric_limits<size_t>::max()) {
        throw logic_error("Nao foi possivel cobrir os mintermos.");
    }

    return melhor;
}

void imprimirSolucao(
    const vector<Termo>& primos,
    const ResultadoCobertura& resultado
) {
    cout << "\nImplicantes selecionados: ";

    for (size_t indice : resultado.indices) {
        cout << "P" << indice << " ";
    }

    cout << "\nEquacao otimizada: ";

    if (resultado.indices.empty()) {
        cout << "0";
    } else {
        for (size_t i = 0; i < resultado.indices.size(); ++i) {
            if (i > 0) {
                cout << " + ";
            }

            cout << termoParaSOP(primos[resultado.indices[i]]);
        }
    }

    cout << "\nNumero de implicantes: "
         << resultado.indices.size();

    cout << "\nNumero de literais: "
         << resultado.literais << '\n';
}

// Reune os mintermos representados por uma lista de termos.
set<int> reunirMintermos(const vector<Termo>& termos) {
    set<int> resultado;

    for (const Termo& termo : termos) {
        resultado.insert(
            termo.mintermos.begin(),
            termo.mintermos.end()
        );
    }

    return resultado;
}

// Obtem os mintermos cobertos pela solucao selecionada.
set<int> coberturaDaSolucao(
    const vector<Termo>& primos,
    const ResultadoCobertura& resultado
) {
    set<int> cobertura;

    for (size_t indice : resultado.indices) {
        cobertura.insert(
            primos[indice].mintermos.begin(),
            primos[indice].mintermos.end()
        );
    }

    return cobertura;
}

// Conta os literais dos produtos interpretados.
// Adequado para as referencias fornecidas no arquivo.
int contarLiteraisDosProdutos(const vector<Produto>& produtos) {
    int total = 0;

    for (const Produto& produto : produtos) {
        for (int valor : produto) {
            if (valor != -1) {
                ++total;
            }
        }
    }

    return total;
}

bool linhaVazia(const string& linha) {
    return linha.find_first_not_of(" \t\r\n") == string::npos;
}


int main(int argc, char* argv[]) {
    string caminho;

    if (argc >= 2) {
        caminho = argv[1];
    } else {
        cout << "Caminho do arquivo de entrada: ";
        getline(cin, caminho);
    }

    ifstream arquivo(caminho);

    if (!arquivo.is_open()) {
        cerr << "Erro: nao foi possivel abrir: "
             << caminho << '\n';
        return 1;
    }

    string linha;
    int numeroLinha = 0;
    int total = 0;
    int processados = 0;
    while (getline(arquivo, linha)) {
        ++numeroLinha;


        // Ignora somente linhas vazias.
        if (linha.find_first_not_of(" \t\r\n") == string::npos) {
            continue;
        }

        ++total;

        cout << "\n========================================\n";
        cout << "CASO " << total
             << "  linha " << numeroLinha << '\n';
        cout << "Entrada: " << linha << '\n';

        try {
            vector<Produto> produtos = lerSOP(linha);
            vector<Termo> mintermos = listarMintermos(produtos);

            cout << "\nMintermos da funcao:\n";

            for (const Termo& termo : mintermos) {
                cout << termo.bits << " ";
                imprimirMintermos(termo.mintermos);
                cout << '\n';
            }

            if (mintermos.empty()) {
                cout << "\nEquacao otimizada: 0\n";
                cout << "Numero de implicantes: 0\n";
                cout << "Numero de literais: 0\n";

                ++processados;
                continue;
            }

            vector<Termo> primos = executarEtapa1(mintermos);

            cout << "\nImplicantes primos:\n";

            for (size_t i = 0; i < primos.size(); ++i) {
                cout << "P" << i << ": "
                     << primos[i].bits << " ";

                imprimirMintermos(primos[i].mintermos);
                cout << '\n';
            }

            imprimirTabelaCobertura(primos, mintermos);

            ResultadoCobertura resultado =
                selecionarCoberturaMinima(primos, mintermos);

            imprimirSolucao(primos, resultado);

            ++processados;
        } catch (const exception& erro) {
            cerr << "Erro na linha " << numeroLinha
                 << ": " << erro.what() << '\n';

        }
    }

    cout << "\n============ RESUMO ============\n";
    cout << "Expressoes encontradas: " << total << '\n';
    cout << "Processadas: " << processados << '\n';

    if (total == 0) {
        cerr << "O arquivo nao contem expressoes.\n";
        return 1;
    }

}
