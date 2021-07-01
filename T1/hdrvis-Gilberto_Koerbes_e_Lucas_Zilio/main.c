/////------GRUPO T1S-17 ------/////
/////------Gilberto Luis Koerbes Junior ------/////
/////------Luca Zilio de Oliveira ------/////
/////////////////////////////////////////
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
//código de referencia MIN e MAX obtido em https://stackoverflow.com/questions/3437404/min-and-max-in-c 
/////////////////////////////////////////

#include <math.h>
#include <string.h> // para usar strings

// Rotinas para acesso da OpenGL
#include "opengl.h"


// Protótipos
void process();
void carregaHeader(FILE *fp);
void carregaImagem(FILE *fp, int largura, int altura);
void criaImagensTeste();

//
// Variáveis globais a serem utilizadas (NÃO ALTERAR!)
//

// Dimensões da imagem de entrada
int sizeX, sizeY;

// Header da imagem de entrada
unsigned char header[11];

// Pixels da imagem de ENTRADA (em formato RGBE)
unsigned char *image;

// Pixels da imagem de SAÍDA (em formato RGB)
unsigned char *image8;

// Fator de exposição
float exposure;

// Histogramas
float histogram[HISTSIZE];
float adjusted[HISTSIZE];

// Flag para exibir/ocultar histogramas
unsigned char showhist = 0;

// Níveis mínimo/máximo (preto/branco)
int minLevel = 0;
int maxLevel = 255;

// Função principal de processamento: ela deve chamar outras funções
// quando for necessário (ex: algoritmo de tone mapping, etc)
void process()
{

    float *fpixels = malloc(sizeX * sizeY * 3 * sizeof(float)); //array apara alocar RGB's em Float
    int tamImage = sizeX * sizeY * 4;                           //image entrada
    int totalBytes = sizeX * sizeY * 3;                         // RGB = 3 bytes por pixel  imagem saida
    unsigned char *ptrImg = image;                              //ponteiro para o vetor imagem
    float mantissa = 0.0;
    float *bkp;
    bkp = fpixels;
    float *ptrE = fpixels; // Exposure
    float *ptrM = fpixels; // Mapping
    float *ptrG = fpixels; // Gama
    /////////////////MANTISSA/////////////////////////
    for (int i = 0; i < tamImage; i += 4)
    {
        float mantissa = pow(2, image[i + 3] - 136);
        float v1 = image[i] * mantissa;
        float v2 = image[i + 1] * mantissa;
        float v3 = image[i + 2] * mantissa;
        *fpixels = v1;
        *fpixels++;
        *fpixels = v2;
        *fpixels++;
        *fpixels = v3;
        *fpixels++;
    }
    fpixels = bkp;
    
    /////////////////EXPOSURE/////////////////////////
    //printf("\n Exposure: %.3f", exposure);
    float expos = pow(2, exposure);
    unsigned char *ptr = image8;
    unsigned char *ptrBit = image8;

    //////////////////////////MAPPING-GAMA-24bits//////////////////////////
    for (int pos = 0; pos < totalBytes; pos++)
    { //unico 'for' realiza as tres operações de forma estruturada
        *fpixels = (*ptrE++ * expos);

        //////////////////MAPPING////////////////////////
        float rgb = (*ptrM) * 0.6;
        float rgbResult = ((rgb) * (2.51 * (rgb) + 0.03)) / ((rgb) * (2.43 * (rgb) + 0.59) + 0.14);

        if (rgbResult > 1)
            rgbResult = 1;
        if (rgbResult < 0)
            rgbResult = 0;
        *ptrM++;

        //////////////////GAMA////////////////////////
        float gama = (1 / 1.8);
        gama = pow((*ptrG), gama);
        *fpixels = gama;
        *ptrG++;

        //////////////////24bits////////////////////////
        float rgb8 = (*fpixels) * 255;
        if (rgb8 > 255)
            rgb8 = 255;
        if (rgb8 < 0)
            rgb8 = 0;

        *ptrBit = (unsigned char)(rgb8);
        *ptrBit++;
        *fpixels++;
    }

    //////INICIA HISTOGRAMA E AJUSTED/////
    for (int i = 0; i < HISTSIZE; i++)
    {
        adjusted[i] = 0;
        histogram[i] = 0;
    }
    /////////////////HISTOGRAMA///////////////////////
    fpixels = bkp; // fpixels agora servira de suporte para guardar I='intensidade' que devera ser usado nos calculos de AJUSTED
    unsigned char *ptrRGB = image8;
    for (int j = 0; j < totalBytes; j += 3)
    {
        float I; //dividimos a formula em partes
        I = (0.299 * ((float)(*ptrRGB)));
        *ptrRGB++;

        I += (0.587 * ((float)(*ptrRGB)));
        *ptrRGB++;

        I += (0.114 * ((float)(*ptrRGB)));
        *ptrRGB++;
        int Iaux = (int)(I);

        histogram[Iaux]++;
        *fpixels++ = I; // fpixles guarda o valor da Intensidade do pixel (os calos de r + g + b)
    }
    //encontra o maior
    float maiorEncontrado = 0;
    for (int i = 0; i < HISTSIZE; i++)
    {
        if (histogram[i] > maiorEncontrado)
        {
            maiorEncontrado = histogram[i];
        }
    }
    //normalizar
    for (int i = 0; i < HISTSIZE; i++)
    {
        histogram[i] = histogram[i] / maiorEncontrado;
        if (histogram[i] > 1)
            histogram[i] = 1;
        if (histogram[i] < 0)
            histogram[i] = 0;
    }

    //////////////////////AJUSTED-preto&branco////////////////////
    ptrRGB = image8;
    fpixels = bkp; //recupera o inicio de fpxiels com os valores de Intensidade
    for (int j = 0; j < totalBytes; j += 3)
    {
        float Ia = (MIN(1, (MAX(0, (*fpixels) - minLevel)) / (maxLevel - minLevel))) * 255;// MIN e MAX em #define

        float RBG = ((*ptrRGB) * Ia) / (*fpixels); //Red
        if (RBG > 255)
            RBG = 255;
        if (RBG < 0)
            RBG = 255;
        *ptrRGB = RBG;
        *ptrRGB++;

        RBG = ((*ptrRGB) * Ia) / (*fpixels); //Blue
        if (RBG > 255)
            RBG = 255;
        if (RBG < 0)
            RBG = 255;
        *ptrRGB = RBG;
        *ptrRGB++;

        RBG = ((*ptrRGB) * Ia) / (*fpixels); //Green
        if (RBG > 255)
            RBG = 255;
        if (RBG < 0)
            RBG = 255;
        *ptrRGB = RBG;
        *ptrRGB++;

        //atualizando o novo histograma//
        int IaHistogramAux = (int)(Ia);
        adjusted[IaHistogramAux]++;

        *fpixels++; //proximo valor do array de intensidade
    }

    //encontra o maior de ajusted
    float maiorEncontradoAjusted = 0;
    for (int i = 0; i < HISTSIZE; i++)
    {
        if (adjusted[i] > maiorEncontradoAjusted)
            maiorEncontradoAjusted = adjusted[i];
    }

    //-----normalizar/calcular novo histograma(ajusted)
    for (int i = 0; i < HISTSIZE; i++)
    {
        adjusted[i] = adjusted[i] / maiorEncontradoAjusted;
        if (adjusted[i] > 1)
            adjusted[i] = 1;
        if (adjusted[i] < 0)
            adjusted[i] = 0;
    }
    fpixels = bkp;//reseta o ponteiro fpixels
    free(fpixels);
    buildTex();
}

int main(int argc, char **argv)
{

    if (argc == 1)
    {
        printf("hdrvis [image file.hdf]\n");
        exit(1);
    }

    // Inicialização da janela gráfica
    init(argc, argv);

    //
    // PASSO 1: Leitura da imagem
    // A leitura do header já foi feita abaixo
    //
    FILE *arq = fopen(argv[1], "rb");
    carregaHeader(arq);

    sizeX = (header[3] * 1) + (header[4] * 256) + (header[5] * 65536) + (header[6] * 4294967296);  // le largura
    sizeY = (header[7] * 1) + (header[8] * 256) + (header[9] * 65536) + (header[10] * 4294967296); // le altura
    printf("Largura %d  x  Altura %d\n", sizeX, sizeY);
    carregaImagem(arq, sizeX, sizeY);

    // Fecha o arquivo
    fclose(arq);

    //
    // COMENTE a linha abaixo quando a leitura estiver funcionando!
    // (caso contrário, ele irá sobrepor a imagem carregada com a imagem de teste)
    //
    //criaImagensTeste();

    exposure = 0.0f; // exposição inicial

    // Aplica processamento inicial
    process();

    // Não retorna... a partir daqui, interação via teclado e mouse apenas, na janela gráfica

    // Mouse wheel é usada para aproximar/afastar
    // Setas esquerda/direita: reduzir/aumentar o fator de exposição
    // A/S: reduzir/aumentar o nível mínimo (black point)
    // K/L: reduzir/aumentar o nível máximo (white point)
    // H: exibir/ocultar o histograma
    // ESC: finalizar o programa

    glutMainLoop();

    return 0;
}

// Função apenas para a criação de uma imagem em memória, com o objetivo
// de testar a funcionalidade de exibição e controle de exposição do programa
void criaImagensTeste()
{
    // TESTE: cria uma imagem de 800x600
    sizeX = 800;
    sizeY = 600;

    printf("%d x %d\n", sizeX, sizeY);

    // Aloca imagem de entrada (32 bits RGBE)
    image = (unsigned char *)malloc(sizeof(unsigned char) * sizeX * sizeY * 4);

    // Aloca memória para imagem de saída (24 bits RGB)
    image8 = (unsigned char *)malloc(sizeof(unsigned char) * sizeX * sizeY * 3);
}

// Esta função deverá ser utilizada para ler o conteúdo do header
// para a variável header (depois você precisa extrair a largura e altura da imagem desse vetor)
void carregaHeader(FILE *fp)
{
    // Lê 11 bytes do início do arquivo
    fread(header, 11, 1, fp);
    // Exibe os 3 primeiros caracteres, para verificar se a leitura ocorreu corretamente
    printf("Id: %c%c%c\n", header[0], header[1], header[2]);
}

// Esta função deverá ser utilizada para carregar o restante
// da imagem (após ler o header e extrair a largura e altura corretamente)
void carregaImagem(FILE *fp, int largura, int altura)
{
    sizeX = largura;
    sizeY = altura;

    // Aloca imagem de entrada (32 bits RGBE)
    image = (unsigned char *)malloc(sizeof(unsigned char) * sizeX * sizeY * 4);

    // Aloca memória para imagem de saída (24 bits RGB)
    image8 = (unsigned char *)malloc(sizeof(unsigned char) * sizeX * sizeY * 3);

    // Lê o restante da imagem de entrada
    fread(image, sizeX * sizeY * 4, 1, fp);
    // Exibe primeiros 3 pixels, para verificação
    for (int i = 0; i < 12; i += 4)
    {
        printf("%02X %02X %02X %02X\n", image[i], image[i + 1], image[i + 2], image[i + 3]);
    }
}
