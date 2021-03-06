#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Para usar strings

#ifdef WIN32
#include <windows.h> // Apenas para Windows
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>   // Funções da OpenGL
#include <GL/glu.h>  // Funções da GLU
#include <GL/glut.h> // Funções da FreeGLUT
#endif

// SOIL é a biblioteca para leitura das imagens
#include <SOIL.h>

// Um pixel RGB (24 bits)
typedef struct
{
    unsigned char r, g, b;
} RGB8;

// Uma imagem RGB
typedef struct
{
    int width, height;
    RGB8 *img;
} Img;

// Protótipos
void load(char *name, Img *pic);
void uploadTexture();
void seamcarve(int targetWidth); // executa o algoritmo
void freemem();                  // limpa memória (caso tenha alocado dinamicamente)

// Funções da interface gráfica e OpenGL
void init();
void draw();
void keyboard(unsigned char key, int x, int y);
void arrow_keys(int a_keys, int x, int y);

// Largura e altura da janela
int width, height;

// Largura desejada (selecionável)
int targetW;

// Identificadores de textura
GLuint tex[3];

// As 3 imagens
Img pic[3];
Img *source;
Img *mask;
Img *target;

// Imagem selecionada (0,1,2)
int sel;

// Carrega uma imagem para a struct Img
void load(char *name, Img *pic)
{
    int chan;
    pic->img = (RGB8 *)SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_RGB);
    if (!pic->img)
    {
        printf("SOIL loading error: '%s'\n", SOIL_last_result());
        exit(1);
    }
    printf("Load: %d x %d x %d\n", pic->width, pic->height, chan);
}

//
// Implemente AQUI o seu algoritmo

void seamcarve(int targetWidth)
{   
    Img *sourceCopy = source;
    int altura = source->height;
    int larguraXY = source->width ;
    int larguraRBG = source->width * 3; //largura para RBG
    unsigned char MatrizRBG[altura][larguraRBG];

    for (int i = 0; i < altura; i++)
    {
        for (int j = 0; j < larguraRBG; )
        {
            MatrizRBG[i][j] = source->img->r;
            MatrizRBG[i][j+1] = source->img->g;
            MatrizRBG[i][j+2] = source->img->b;
            source->img++;
            j = j+3;
        }
        //printf("\n i = %d", i);        
    }
    
    
    int Energy[altura][larguraXY];

    boolean bordaXLeft = FALSE;
    boolean bordaXRight = FALSE;
    boolean bordaYUp = FALSE;
    boolean bordaYDown = FALSE;

for (int i = 0; i < altura; i++)
    {   
        unsigned char RUp;
        unsigned char GUp;
        unsigned char BUp;
        unsigned char RDown;
        unsigned char GDown;
        unsigned char BDown;

        unsigned char RLeft;
        unsigned char GLeft;
        unsigned char BLeft;
        unsigned char RRight;
        unsigned char GRight;
        unsigned char BRight;

        


        if(i == 0 ){ bordaYUp = TRUE;}
        else if (i == altura) {bordaYDown = TRUE;}        
        int k =0;
        for (int j = 0; j < larguraRBG; )
        {
            if(j == 0 ){ bordaXLeft = TRUE;}
            else if (j == larguraRBG-3) {bordaXRight = TRUE;}   
            int R;
            int G;
            int B;
            int energia;
                   
            //Para X
            if(bordaXLeft){
                RLeft = MatrizRBG[i][larguraRBG-3];
                GLeft = MatrizRBG[i][larguraRBG-2];
                BLeft = MatrizRBG[i][larguraRBG-1];
                RRight = MatrizRBG[i][j+3];
                GRight = MatrizRBG[i][j+4];
                BRight = MatrizRBG[i][j+5];
            }else if(bordaXRight){
                RLeft = MatrizRBG[i][j-3];
                GLeft = MatrizRBG[i][j-4];
                BLeft = MatrizRBG[i][j-5];
                RRight =MatrizRBG[i][0];
                GRight =MatrizRBG[i][1]; 
                BRight =MatrizRBG[i][2]; 

            }else{
                RLeft = MatrizRBG[i][j-3];
                GLeft = MatrizRBG[i][j-4];
                BLeft = MatrizRBG[i][j-5];
                RRight =MatrizRBG[i][j+3];
                GRight =MatrizRBG[i][j+4]; 
                BRight =MatrizRBG[i][j+5];

            }
            R = RRight - RLeft;
            G = GRight - GLeft;
            B = BRight - BLeft;
            energia = pow(R,2) + pow(G,2)+ pow(B,2);
            //Para Y
            if(bordaYUp){
                RUp = MatrizRBG[altura][j];
                GUp = MatrizRBG[altura][j+1];
                BUp = MatrizRBG[altura][j+2];
                RDown = MatrizRBG[i+1][j];
                GDown = MatrizRBG[i+1][j+1];
                BDown = MatrizRBG[i+1][j+2];
            }else if(bordaYDown){
                RUp = MatrizRBG[i-1][j];
                GUp = MatrizRBG[i-1][j+1];
                BUp = MatrizRBG[i-1][j+2];
                RDown = MatrizRBG[0][j];
                GDown = MatrizRBG[0][j+1];
                BDown = MatrizRBG[0][j+2];               
            }else{
                RUp = MatrizRBG[i-1][j];
                GUp = MatrizRBG[i-1][j+1];
                BUp = MatrizRBG[i-1][j+2];
                RDown = MatrizRBG[i+1][j];
                GDown = MatrizRBG[i+1][j+1];
                BDown = MatrizRBG[i+1][j+2];
            }
            //calcular
            R = RUp - RDown;
            G = GUp = GDown;
            B = BUp - GDown;
            //printf("\n Energia 1 Pre Soma  = %d", energia);
            //int aux = (pow(R,2) + pow(G,2)+ pow(B,2));
            //printf("\n Energia 2 Pre Soma = %d", aux);
            energia = energia + (pow(R,2) + pow(G,2)+ pow(B,2));
            //printf("\n Energia Pos Soma = %d", energia);
            Energy[i][k] = energia;
            //printf("\nValor gravado = %d", Energy[i][k]);


            j = j+3;
            k++;
        }
        
    }

    //===========================Procurando caminho
    int EnergiaAcumulada[larguraXY];//dois vetores possui ligação entre seus valores, especie de MAP
    int PosFinal[larguraXY];           //dois vetores possui ligação entre seus valores, especie de MAP
    for (int j = 0;j < larguraXY; j++)
    {
        int EnergiaAcumuladaCaminho=0;
        int Direction = 0;
        for (int i = 0; i < altura; i++)
        {
            int atualEnergy = MatrizRBG[i][j];
            int D = MatrizRBG[i][j];;
            int R = MatrizRBG[i][j];;
            int L = MatrizRBG[i][j];;
            if(Direction==0){//verifica bordas
                D = MatrizRBG[i+1][Direction];  
                R = MatrizRBG[i+1][Direction+1];
                if(D<atualEnergy) {Direction = Direction; EnergiaAcumuladaCaminho += D;}
                else if(D<atualEnergy) {Direction = Direction+1; EnergiaAcumuladaCaminho += R;}

            }else if(Direction==larguraXY-1){ //verifica bordas
                D = MatrizRBG[i+1][Direction];                
                L = MatrizRBG[i+1][Direction-1];
                if(D<atualEnergy) {Direction = Direction;EnergiaAcumuladaCaminho += D;}
                else if(L<atualEnergy) {Direction = Direction-1; EnergiaAcumuladaCaminho += L;}
            }else{
                D = MatrizRBG[i+1][Direction];
                R = MatrizRBG[i+1][Direction+1];
                L = MatrizRBG[i+1][Direction-1];

                if(D<atualEnergy) {Direction = Direction;EnergiaAcumuladaCaminho += D;}
                else if(L<atualEnergy) {Direction = Direction-1; EnergiaAcumuladaCaminho += L;}
                else {Direction = Direction; EnergiaAcumuladaCaminho += D;}
            }
            EnergiaAcumulada[j]=EnergiaAcumuladaCaminho;
            PosFinal[j]=Direction;    
        }
    }
    printf("\n Rodando");
    source = sourceCopy;    

 




    
    





    // Aplica o algoritmo e gera a saida em target->img...

    RGB8(*ptr)
    [target->width] = (RGB8(*)[target->width])target->img;

    for (int y = 0; y < target->height; y++)
    {
        for (int x = 0; x < targetW; x++)
            ptr[y][x].r = ptr[y][x].g = 255;
        for (int x = targetW; x < target->width; x++)
            ptr[y][x].r = ptr[y][x].g = 0;
    }
    // Chame uploadTexture a cada vez que mudar
    // a imagem (pic[2])
    uploadTexture();
    glutPostRedisplay();
}

void freemem()
{
    // Libera a memória ocupada pelas 3 imagens
    free(pic[0].img);
    free(pic[1].img);
    free(pic[2].img);
}

/********************************************************************
 * 
 *  VOCÊ NÃO DEVE ALTERAR NADA NO PROGRAMA A PARTIR DESTE PONTO!
 *
 ********************************************************************/
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("seamcarving [origem] [mascara]\n");
        printf("Origem é a imagem original, mascara é a máscara desejada\n");
        exit(1);
    }
    glutInit(&argc, argv);

    // Define do modo de operacao da GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // pic[0] -> imagem original
    // pic[1] -> máscara desejada
    // pic[2] -> resultado do algoritmo

    // Carrega as duas imagens
    load(argv[1], &pic[0]);
    load(argv[2], &pic[1]);

    if (pic[0].width != pic[1].width || pic[0].height != pic[1].height)
    {
        printf("Imagem e máscara com dimensões diferentes!\n");
        exit(1);
    }

    // A largura e altura da janela são calculadas de acordo com a maior
    // dimensão de cada imagem
    width = pic[0].width;
    height = pic[0].height;

    // A largura e altura da imagem de saída são iguais às da imagem original (1)
    pic[2].width = pic[1].width;
    pic[2].height = pic[1].height;

    // Ponteiros para as structs das imagens, para facilitar
    source = &pic[0];
    mask = &pic[1];
    target = &pic[2];

    // Largura desejada inicialmente é a largura da janela
    targetW = target->width;

    // Especifica o tamanho inicial em pixels da janela GLUT
    glutInitWindowSize(width, height);

    // Cria a janela passando como argumento o titulo da mesma
    glutCreateWindow("Seam Carving");

    // Registra a funcao callback de redesenho da janela de visualizacao
    glutDisplayFunc(draw);

    // Registra a funcao callback para tratamento das teclas ASCII
    glutKeyboardFunc(keyboard);

    // Registra a funcao callback para tratamento das setas
    glutSpecialFunc(arrow_keys);

    // Cria texturas em memória a partir dos pixels das imagens
    tex[0] = SOIL_create_OGL_texture((unsigned char *)pic[0].img, pic[0].width, pic[0].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    tex[1] = SOIL_create_OGL_texture((unsigned char *)pic[1].img, pic[1].width, pic[1].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Exibe as dimensões na tela, para conferência
    printf("Origem  : %s %d x %d\n", argv[1], pic[0].width, pic[0].height);
    printf("Máscara : %s %d x %d\n", argv[2], pic[1].width, pic[0].height);
    sel = 0; // pic1

    // Define a janela de visualizacao 2D
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, width, height, 0.0);
    glMatrixMode(GL_MODELVIEW);

    // Aloca memória para a imagem de saída
    pic[2].img = malloc(pic[1].width * pic[1].height * 3); // W x H x 3 bytes (RGB)
    // Pinta a imagem resultante de preto!
    memset(pic[2].img, 0, width * height * 3);

    // Cria textura para a imagem de saída
    tex[2] = SOIL_create_OGL_texture((unsigned char *)pic[2].img, pic[2].width, pic[2].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Entra no loop de eventos, não retorna
    glutMainLoop();
}

// Gerencia eventos de teclado
void keyboard(unsigned char key, int x, int y)
{
    if (key == 27)
    {
        // ESC: libera memória e finaliza
        freemem();
        exit(1);
    }
    if (key >= '1' && key <= '3')
        // 1-3: seleciona a imagem correspondente (origem, máscara e resultado)
        sel = key - '1';
    if (key == 's')
    {
        seamcarve(targetW);
    }
    glutPostRedisplay();
}

void arrow_keys(int a_keys, int x, int y)
{
    switch (a_keys)
    {
    case GLUT_KEY_RIGHT:
        if (targetW <= pic[2].width - 10)
            targetW += 10;
        seamcarve(targetW);
        break;
    case GLUT_KEY_LEFT:
        if (targetW > 10)
            targetW -= 10;
        seamcarve(targetW);
        break;
    default:
        break;
    }
}
// Faz upload da imagem para a textura,
// de forma a exibi-la na tela
void uploadTexture()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 target->width, target->height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, target->img);
    glDisable(GL_TEXTURE_2D);
}

// Callback de redesenho da tela
void draw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Preto
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Para outras cores, veja exemplos em /etc/X11/rgb.txt

    glColor3ub(255, 255, 255); // branco

    // Ativa a textura corresponde à imagem desejada
    glBindTexture(GL_TEXTURE_2D, tex[sel]);
    // E desenha um retângulo que ocupa toda a tela
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2f(0, 0);

    glTexCoord2f(1, 0);
    glVertex2f(pic[sel].width, 0);

    glTexCoord2f(1, 1);
    glVertex2f(pic[sel].width, pic[sel].height);

    glTexCoord2f(0, 1);
    glVertex2f(0, pic[sel].height);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Exibe a imagem
    glutSwapBuffers();
}
