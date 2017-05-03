#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <locale.h>
#include <time.h>
#include <ctype.h>

#define MAX_CHAR_NOME_EVENTO 80
#define MAX_CHAR_BUSCA 30



#define DIREITA 1
#define BAIXO 2
#define CIMA 4
#define ESQUERDA 3





// ============= funçoes para interface ============================

typedef struct {
    COORD lsup;
    COORD linf;
    char * texto_salvo;
}bloco_tela;

/*
bloco_tela gravaTela(COORD lsup, COORD linf){
    bloco_tela ret;
    ret.lsup = lsup;
    ret.linf = linf;
    ret.texto_salvo = malloc((linf.X-lsup.X+1)*(linf.Y-lsup.Y+1));
    for (i=0;i<ncarac;i++)
        texto[i] = get_char_at_xy(inicio.X+i,inicio.Y);
        texto[i]='\0';
}
*/


char get_char_at_xy( int x, int y )
  {
  CHAR_INFO ci;
  COORD xy = { 0, 0 };
  SMALL_RECT rect = { x, y, x, y };
  return ReadConsoleOutput( GetStdHandle( STD_OUTPUT_HANDLE ), &ci, (COORD){1,1}, xy, &rect )
       ? ci.Char.AsciiChar
       : '\0';
  }

void gotoxy(int x, int y){
     SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),(COORD){x,y});
}

void escreveTexto(char * texto, COORD inicio, int tam_linha){
    int i=0,j=0;
    gotoxy(inicio.X, inicio.Y);
    for(;texto[i]!='\0';i++,j++){
        if(j==tam_linha){
            j=0;
            inicio.Y++;
            gotoxy(inicio.X,inicio.Y);
        }
        putchar(texto[i]);
    }
}

void entra_texto(COORD inicio, int tam, char *texto){
    int ox = inicio.X;
    int oy = inicio.Y;
    char c_atual;
    int ncarac=0,i=0;
    gotoxy(ox , oy);
    for(;i<tam;i++) putchar(' ');
    gotoxy(ox,oy);
    while(1){
        c_atual = getch();
        switch (c_atual){
            case 13:
                for (i=0;i<ncarac;i++)
                    texto[i] = get_char_at_xy(inicio.X+i,inicio.Y);
                    texto[i]='\0';
                    return;
            case 8:
                if (ncarac > 0){
                   putchar(8);
                   putchar(' ');
                   putchar(8);
                   ncarac--;
                }
                break;
            default:
                if(ncarac < tam && ( (isalnum(c_atual)) || c_atual == 32)){
                    putchar(c_atual);
                    ncarac++;
                }
        }
    }
}





// =================================== FUNÇOES DE SELECAO =========================================


typedef struct{
    char carac[3];
    COORD origem;
    COORD mn;
    COORD offset;
    COORD limites;
    int dist;
}mCursor;


void desenha_cursor(mCursor curs){
    gotoxy( curs.origem.X + curs.offset.X * curs.mn.X, curs.origem.Y + curs.offset.Y * curs.mn.Y);
    putchar(curs.carac[0]);
    gotoxy(curs.origem.X + curs.offset.X * curs.mn.X + curs.dist, curs.origem.Y + curs.offset.Y * curs.mn.Y);
    putchar(curs.carac[1]);
}

void apaga_cursor(mCursor curs){
    gotoxy( curs.origem.X + curs.offset.X * curs.mn.X, curs.origem.Y + curs.offset.Y * curs.mn.Y);
    putchar(' ');
    gotoxy(curs.origem.X + curs.offset.X * curs.mn.X + curs.dist, curs.origem.Y + curs.offset.Y * curs.mn.Y);
    putchar(' ');
}


COORD move_cursor(mCursor *curs,int direcao){
switch (direcao){
    case DIREITA:
        if(curs->limites.X>curs->mn.X) curs->mn.X++;
        break;
    case ESQUERDA:
        if(0<curs->mn.X) curs->mn.X--;
        break;
    case CIMA:
        if(0<curs->mn.Y) curs->mn.Y--;
        break;
    case BAIXO:
        if(curs->limites.Y>curs->mn.Y) curs->mn.Y++;
        break;
    }
    return curs->mn;
}

void criaCursor(mCursor* curs, int dist, COORD origem, COORD limites, COORD offset){
    curs->dist = dist;
    curs->limites = limites;
    curs->origem = origem;
    curs->offset = offset;
    curs->carac[0]='[';
    curs->carac[1]=']';
    curs->mn = (COORD){0,0};
}


// ============================= Funcoes de data ===================================================

typedef struct tm tm;


int numero_dias_mes(int mes , int ano){
            int dias;
            if (mes == 4 || mes == 6 || mes == 9 || mes == 11)
              dias = 30;
            else if (mes == 2)
            { int ehBissexto = (ano % 4 == 0 && ano % 100 != 0) || (ano % 400 == 0);
              if (ehBissexto)
                dias = 29;
              else
                dias = 28;
            }
            else
              dias = 31;
            return dias;
}

int valida_data(tm data){
    if(numero_dias_mes(data.tm_mon,data.tm_year) < data.tm_mday || data.tm_mday < 1 || data.tm_mon > 12 || data.tm_mon < 1)return 0;
    if(data.tm_hour <0 || data.tm_hour > 23 || data.tm_min < 0 || data.tm_min> 59)return 0;
    return 1;
}

int dia_semana(int d, int m , int a){
    return (d += m < 3 ? a-- : a - 2, 23*m/9 + d + 4 + a/4- a/100 + a/400)%7;
}


void desenha_calendario(int mes, int ano,COORD origem){
        gotoxy(origem.X,origem.Y);
        printf("D    S    T    Q    Q    S    S    ");
        origem.Y++;
        gotoxy(origem.X,origem.Y);
        int dia_1 = dia_semana(1,mes,ano);
        int tam_mes = numero_dias_mes(mes,ano);
        int i,j=0;
        for(i=0;i<dia_1;i++,j++)
            printf("     ");
        for(i=1;i<=tam_mes;i++,j++){
            if(j==7){
                origem.Y+=2;
                gotoxy(origem.X,origem.Y);
                j=0;
            }
            if(i<10)printf(" ");
            printf("%d   ",i);
        }
}










//==============================Funçoes de evento===================================================


typedef struct {
    int hora;
    int min;
}t_hora;

typedef struct {
    time_t inicio;
    time_t fim;
    char * nome;
    char * local;
}t_evento;
t_evento *eventos;

typedef struct {
    time_t inicio;
    time_t fim;
    t_hora hora_inicio;
    t_hora hora_fim;
    char * nome;
    char * local;
    int recorrente;
    int dias_semana;
}t_eveRec;

int valida_hora(t_hora hora){
    return ( (hora.hora < 24 && hora.hora >= 0 ) && (hora.min >= 0 && hora.min < 60) );
}



/*
void cria_evento_recorrente (t_evento *evento ,time_t inicio,time_t fim, t_hora hora_inicio , t_hora hora_fim ,char * nome , char * local ,int recorrente, int dias){
    evento->inicio = inicio;
    evento->fim = fim;
    evento->hora_inicio.hora;
    evento->hora_fim.hora;
    evento->nome = malloc(strlen(nome));
    evento->local = malloc(strlen (local));
    strcpy(evento->nome, nome);
    strcpy(evento->local, local);
    evento->recorrente = recorrente;
    evento->dias_semana= dias;
}*/

void cria_evento (t_evento *evento ,time_t inicio,time_t fim, char * nome , char * local ){
    char * name;
    printf(nome);
    name = malloc(strlen(nome));
    evento->inicio = inicio;
    evento->fim = fim;
    evento->nome = name;
    evento->local = malloc(strlen (local));
    strcpy(evento->nome, nome);
    strcpy(evento->local, local);
}



void apaga_evento(t_evento * evento){
    free(evento->nome);
    free(evento->local);
    free(evento);
}
/*
void modifica_evento (t_evento *evento ,time_t inicio,time_t fim, t_hora hora_inicio , t_hora hora_fim ,  char * nome , char * local ,int recorrente, int dias){
    evento->inicio = inicio;
    evento->fim = fim;
    evento->hora_inicio.hora;
    evento->hora_fim.hora;
    realloc(&evento->nome,strlen(nome));
    realloc(&evento->local, strlen (local));
    strcpy(evento->nome, nome);
    strcpy(evento->local, local);
    evento->recorrente = recorrente;
    evento->dias_semana;
}*/

void modifica_evento (t_evento *evento ,time_t inicio,time_t fim, t_hora hora_inicio , t_hora hora_fim ,  char * nome , char * local ,int recorrente, int dias){
    evento->inicio = inicio;
    evento->fim = fim;
    realloc(&evento->nome,strlen(nome));
    realloc(&evento->local, strlen (local));
    strcpy(evento->nome, nome);
    strcpy(evento->local, local);
}


void mfgets(char * texto, int tam){
    fgets(texto, tam, stdin);
    setbuf(stdin, NULL);
}


// ======= FUNÇOES DE BUSCA POR NOME =================

typedef struct{
   char * nome;
   int * posicoes;
   int tam;
}t_indice;

int carrega_indice(t_indice ** listaPalavras){
        int tam , res, elem = 0;
        int espaco = 0;
        char nome[80];
        int posicoes[100];
        t_indice *listaA = * listaPalavras;
        FILE * fpt;
        fpt = fopen("indice.txt", "r");
        listaA = malloc(100 * sizeof(t_indice));
        while(res==2){
            res = fscanf(fpt , "%[^\n]s %d" , &nome , &tam);

            //printf("%s", *eventos[elem]->nome);
            elem++;
            espaco--;
            printf("%d",res);
            printf("*");
            if(espaco == 0){
                espaco = 100;
                realloc(listaA, (/*elem*/ + 100) * sizeof(t_evento));
            }
        }
        fclose(fpt);
        *listaPalavras = listaA;
        return elem;
}




int busca_indice(t_indice * indice , char * palavra , int tamanho){
    int esq, dir, meio, comp;
    while(esq!=dir){
        meio = (esq+dir)/2;
        comp = strcmp(palavra, indice[meio].nome);
        if (comp == 0) return meio;
        if(comp>0) esq = meio;
        else dir = meio;
        }


    }




void insere_indice(t_indice * indice , char * palavra, int tam){

}


/*
//Função Ordena por nome do evento.

void ordena_indice(t_indice *name, int tam) {
    int i, j;
    t_indice aux;
    for(i = 1; i < strlen(name) ; i++) {
        aux = name[i];
        for(j = i; (j>0) && (strcmp(aux.nome, name[j-1].nome) < 0); j--)
            name[j] = name[j-1];
        name[j] = aux;
    }
}*/

//int compara_datas(tm data1, tm data_2){

/*
void entraData(COORD posicao, int * dia , int *mes , int ano){
    int ox = inicio.X;
    int oy = inicio.Y;
    char c_atual , texto[11];
    int ncarac=0,i=0;
    gotoxy(ox , oy);
    printf("  /  /    ");
    gotoxy(ox,oy);
    while(1){
        c_atual = getch();
        switch (ncarac){
        case 0:
            gotoxy()
        case 1:


        case 2:


        case 3:
        }
        switch (c_atual){
            case 13:
                for (i=0;i<10;i++)
                    texto[i] = get_char_at_xy(inicio.X+i,inicio.Y);
                    texto[2]='\0';
                    texto[5]='\0';
                    texto[9]='\0';
                    dia = atoi(&texto);
                    mes = atoi(&texto[3])
                    ano = atoi(&texto[6])
                    return;
            case 8:
                if (ncarac > 0){
                   putchar(8);
                   putchar(' ');
                   putchar(8);
                   ncarac--;
                }
                break;
            default:
                if(ncarac < tam && ( (isalnum(c_atual)) || c_atual == 32)){
                    putchar(c_atual);
                    ncarac++;
                }
        }
    }
}
*/






//==================== FUNÇOES DE ARQUIVO =====================

void grava_eventos(t_evento * eventos,int tam){
    FILE * fpt;
    int i;
    fpt = fopen("eventos.txt", "w");
    for(i = 0; i< tam; i++){
        fprintf(fpt , "%lu %lu %d %d %d %s%s" , eventos->inicio , eventos->fim , eventos->nome , eventos->local);
        eventos = &eventos[1];
    }
    fclose(fpt);
}

int carrega_eventos(t_evento** eventos){
    int res=6, elem = 0 , espaco=100;
    time_t inicio , fim ;
    t_hora hora_inicio;
    t_hora hora_fim;
    t_evento * eventoA = * eventos;
    int recorrente , dias_semana;
    char  nome [81] , local [81];
    FILE * fpt;
    fpt = fopen("eventos.txt", "r");
    eventoA = malloc(100 * sizeof(t_evento));
    while(res==6){
        res = fscanf(fpt , "%lu %lu %[^\n]s ^\n %[^\n]s" , &inicio , &fim ,  nome , local);
        cria_evento(&eventoA[elem] , inicio , fim , nome , local);
        //printf("%s", *eventos[elem]->nome);
        elem++;
        espaco--;
        printf("%d",res);
        printf("*");
        if(espaco == 0){
            espaco = 100;
            realloc(eventoA, (/*elem*/ + 100) * sizeof(t_evento));
        }
    }
    fclose(fpt);
    *eventos = eventoA;
    return elem;
}


int lista_eventos(t_evento * eventos, tm data ,int tam){
    struct tm tm_hora_zero, tm_hora_23;
    tm_hora_zero = data;
    tm_hora_23 = data;

    tm_hora_zero.tm_hour = 0;
    tm_hora_zero.tm_min = 0;
    tm_hora_zero.tm_sec = 0;
    tm_hora_23.tm_hour = 23;
    tm_hora_23.tm_min = 59;
    tm_hora_23.tm_sec = 59;
    time_t hora_zero;
    int aux = mktime(&tm_hora_zero);
    time_t hora_meiaNoite;
    int temp = time(&hora_meiaNoite);
    int i =0;
    double difft;
    time(&hora_zero);
    time(&hora_meiaNoite);
    difft = difftime(hora_zero, eventos[i].fim);
    printf("%f" , difft);
    return 0;
    for(; i<tam; i++){
        if((eventos[i].inicio) <= hora_meiaNoite && (eventos[i].fim) >= hora_zero);


    }

}


// =========================== FUNÇOES DE INTERFACE PARTE 2 ========================

void insere_evento_arry (t_evento ** eventos , t_evento novoEvento, int tam){
    int i;
    t_evento * eventosA;
    eventosA = *eventos;
    int posicao = busca_data(eventosA , novoEvento.inicio, tam);
    eventosA = realloc(eventosA , (tam+1) * sizeof(t_evento));
    for(i=tam ; tam>posicao; tam--){
        eventosA[i-1] = eventosA[i];
    }
    cria_evento(eventosA, novoEvento.inicio,novoEvento.fim, novoEvento.nome , novoEvento.local);
}

void inserir_evento(t_evento * eventos , int tam)
{
    int j=0, k=0;
    char nome_evento[81];
    char descricao[1000];
    int dia1,mes1,ano1;
    int dia2,mes2,ano2;
    int hora1 , min1;
    int hora2, min2;
    t_evento evento_Novo;
    struct tm data;
    time_t data_inicial;
    time_t data_final;
    printf("Insira o nome do evento:");
    mfgets(nome_evento, sizeof(nome_evento));
    printf("Insira a descricao do evento:");
    fgets(descricao, sizeof(descricao), stdin);
    fflush(stdin);
    printf("Data inicial:");
    scanf(" %i %i %i", &dia1 , &mes1, &ano1);
    printf("Data final:");
    scanf(" %i %i %i", &dia2, &mes2, &ano2);
    printf("Hora inicial:");
    scanf(" %i %i", &hora1 , &min1);
    printf("Hora final:");
    scanf(" %i %i", &hora2, &min2);
    data.tm_hour = hora1;
    data.tm_min = min1;
    data.tm_sec = 0;
    data.tm_mday = dia1;
    data.tm_yday = mes1;
    data_inicial = mktime(&data);
    data.tm_hour = hora2;
    data.tm_min = min2;
    data.tm_sec = 0;
    data.tm_mday = dia2;
    data.tm_yday = mes2;
    data_final = mktime(&data);
    insere_evento_arry(&eventos, evento_Novo , tam);
}

int busca_data(t_evento * eventos, time_t inicio, int tamanho){
    int e, d, meio;
    e = 0; d = tamanho-1;
    while(e <= d){
        meio = (e+d)/2;
        if(eventos[meio].inicio == inicio){
                return meio;
        }
        if (inicio > eventos[meio].inicio){
            e = meio+1;
        }
        else{
            d = meio-1;
        }

    }
    return meio;
}



void menu()
{
    printf("\t\t\t=====================================================");
    printf("\n\t\t\t\t\t    >>>  Menu  <<<\n");
    printf("\t\t\t=====================================================\n");
    printf("\n1. Inserir um evento");
    printf("\n2. Alterar um evento");
    printf("\n3. Apagar um evento");
    printf("\n4. Listar evento(s)");
    printf("\n5. Buscar evento");
    printf("\n0. Sair\n");
}


int main(){
    FILE *arquivo;
    t_evento *eventos;
    t_evento  *eventos2;
    time_t seila;
    int i=0, * b;
    char nome[50], local[50];
    int resultado_busca[100];
    int tam = carrega_eventos(&eventos);



    int escolha;
    menu();
    scanf("%d", &escolha);
    getchar();
    printf("\n");
    while (escolha != 0)
    {
        switch (escolha)
        {
        case 1:
            inserir_evento(eventos, tam);
            break;
        case 4:

            break;
        default:
            printf("Opcao invalida! Tente novamente.\n");

        }
        menu();
        scanf("%i", &escolha);
        getchar();
    return 0;
}




/*


    carrega_eventos(&eventos2);
system("cls");
    tm dataa;
    time_t teste1;
    dataa.tm_year = 2017;
    dataa.tm_mon = 4;
    dataa.tm_mday = 20;
    dataa.tm_hour = 11;
    dataa.tm_min = 20;

    time(&teste1);
   setlocale(LC_ALL, "");
printf("%ls",L"╔═════════════════════╤════════════════════════╦═══════════════════════════════════════════════════════╗\n║       DEZEMBRO      │         2017           ║                 QUARTA-FEIRA 8                        ║\n╠═════════════════════╧════════════════════════╬═══════════════════════════════════════════════════════╣\n║                                              ║                                                       ║\n║                                              ║                                                       ║\n║                                              ╟                                                       ╢\n║                                              ║                                                       ║\n║                                              ║                                                       ║\n║                                              ╟                                                       ╢\n║                                              ║                                                       ║\n║                                              ║                                                       ║\n║                                              ╟                                                       ╢\n║                                              ║                                                       ║\n║                                              ║                                                       ║\n║                                              ╟                                                       ╢\n║                                              ║                                                       ║\n║                                              ║                                                       ║\n║                                              ║                                                       ║\n║                                              ║                                                       ║\n║                                              ║                                                       ║\n╠══════════════════════════════════════════════║                                                       ║\n║                                              ║                                                       ║\n║ Inicio:                                      ║                                                       ║\n║ Fim:                                         ║                                                       ║\n║ Local:                                       ║                                                       ║\n║                                              ║                                                       ║\n║ Recorrente:                                  ║                                                       ║\n║    De:   /  /     até   /  /                 ║                                                       ║\n║                                              ║                                                       ║\n║                                              ║                                                       ║\n║                                              ║                                                       ║\n╚══════════════════════════════════════════════╩═══════════════════════════════════════════════════════╝\n");

    int a;
    mCursor cursor1;
    criaCursor(&cursor1, 5, (COORD){2,7}, (COORD){6,5}, (COORD){5,2});
    printf("%d %d", cursor1.mn.X,cursor1.mn.Y);
    desenha_calendario(4,2017,(COORD){4,6});
    getch();
    while(1){
        a=getch();

        a=getch();
        apaga_cursor(cursor1);
        if(a==80)move_cursor(&cursor1,BAIXO);
        if(a==72)move_cursor(&cursor1,CIMA);
        if(a==75)move_cursor(&cursor1,ESQUERDA);
        if(a==77)move_cursor(&cursor1,DIREITA);
        desenha_cursor(cursor1);
    }



   // valida_data(tm data)  ====>>> retorna 1 se data for válida, retorna 0 se data for inválida;

 ///   int numeroEventos = carrega_eventos(eventos2);
    printf("==%d==", numeroEventos);
for(i=0;i<numeroEventos;i++)
    //printf("%lu %lu %d %d %s %s" , eventos2[i].inicio , eventos2[i].fim , eventos2[i].recorrente , eventos2[i].dias_semana , eventos2[i].nome , eventos2[i].local);
    printf("%s" , eventos2[i].nome );
    return 0;*/
}
