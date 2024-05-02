/* Integrantes:
 * Guilherme Lorete Schmidt - 13676857
 * Luana Hartmann Franco da Cruz - 13676350
 * João Pedro Gomes - 13839069
 * Lucas Corlete Alves de Melo - 13676461
 * João Victor Breches Alves - 13677142
 */

/* Compilacao:
 * gcc -o main main.c -pthread
 */

/* Execucao:
 * ./main
 * digite as variaveis separadas por espaco na ordem da especificacao
 */

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>

// Funcoes de cada thread, ranks 0 a 5
int criador();
void *deposito_materia();
void *fabrica();
void *controle();
void *deposito_caneta();
void *comprador();


// Variaveis fornecidas na inicializacao (na ordem da especificacao)
int materia_existente;
int materia_por_envio;
int intervalo_envio_materia;
int tempo_fabricacao;
int maximo_canetas_armazendas;
int canetas_por_compra;
int tempo_compra;

// Variaveis pra comunicacao entre threads
int espaco_deposito = 0;
int materia_enviada = 0;
int demanda_caneta = 0;
int caneta_transferida_deposito = 0;
int canetas_solicitadas = 0;
int canetas_transferidas_comprador = 0;
int canetas_compradas = -1;

// Variaveis condicionais necessarias
pthread_cond_t cond_caneta_transferida_deposito;
pthread_cond_t cond_canetas_transferidas_comprador;
pthread_cond_t cond_canetas_solicitadas;
pthread_cond_t cond_informacao_canetas_transferidas;
pthread_cond_t cond_materia_recebida;

// Mutexes necessarios
pthread_mutex_t mutex_materia_enviada;
pthread_mutex_t mutex_caneta_fabricar;
pthread_mutex_t mutex_espaco_deposito;
pthread_mutex_t mutex_caneta_transferida_deposito;
pthread_mutex_t mutex_canetas_transferidas_comprador;
pthread_mutex_t mutex_canetas_solicitadas;
pthread_mutex_t mutex_informacao_canetas_transferidas;

// Thread criador (rank 0)
int main() {

    // Leitura dos argumentos de entrada
    scanf("%d %d %d %d %d %d %d", &materia_existente, &materia_por_envio,
    &intervalo_envio_materia, &tempo_fabricacao, &maximo_canetas_armazendas,
    &canetas_por_compra, &tempo_compra);

    return criador();
}

/**
 * @brief Recebe argumentos de entrada e instancia demais threads do programa
 * @return 0 caso conclua com sucesso, 1 caso fracasse
 */
int criador() {
    // inicializa mutexes
    pthread_mutex_init(&mutex_materia_enviada, NULL);
    pthread_mutex_init(&mutex_caneta_fabricar, NULL);
    pthread_mutex_init(&mutex_espaco_deposito, NULL);
    pthread_mutex_init(&mutex_caneta_transferida_deposito, NULL);
    pthread_mutex_init(&mutex_canetas_solicitadas, NULL);
    pthread_mutex_init(&mutex_canetas_transferidas_comprador, NULL);
    pthread_mutex_init(&mutex_informacao_canetas_transferidas, NULL);
    
    // inicializa variaveis condicionais
    pthread_cond_init(&cond_caneta_transferida_deposito, NULL);
    pthread_cond_init(&cond_canetas_solicitadas, NULL);
    pthread_cond_init(&cond_canetas_transferidas_comprador, NULL);
    pthread_cond_init(&cond_informacao_canetas_transferidas, NULL);

    // Thread para o deposito de materia prima, rank 1
    pthread_t thread_deposito_materia;
    if(pthread_create(&thread_deposito_materia, 0, &deposito_materia, 0) != 0) {
        printf("Erro na inicializacao do deposito de materia prima\n");
        return 1;
    }
    
    // Thread para a fabrica, rank 2
    pthread_t thread_fabrica;
    if(pthread_create(&thread_fabrica, 0, &fabrica, 0) != 0) {
        printf("Erro na inicializacao da fabrica\n");
        return 1;
    }
    
    // Thread para o controle, rank 3
    pthread_t thread_controle;
    if(pthread_create(&thread_controle, 0, &controle, 0) != 0) {
        printf("Erro na inicializacao do controle\n");
        return 1;
    }
    
    // Thread para o deposito de canetas, rank 4
    pthread_t thread_deposito_caneta;
    if(pthread_create(&thread_deposito_caneta, 0, &deposito_caneta, 0) != 0) {
        printf("Erro na inicializacao do deposito de canetas\n");
        return 1;
    }
    
    // Thread para o comprador, rank 5
    pthread_t thread_comprador;
    if(pthread_create(&thread_comprador, 0, &comprador, 0) != 0) {
        printf("Erro na inicializacao do comprador\n");
        return 1;
    }

    // Loop para output da canetas compradas
    int canetas_compradas_local = 0;
    int total = 0;
    while(canetas_compradas_local >= 0){
        // receber informacao sobre canetas compradas
        pthread_mutex_lock(&mutex_informacao_canetas_transferidas);
        while(canetas_compradas == -1)
            pthread_cond_wait(&cond_informacao_canetas_transferidas, &mutex_informacao_canetas_transferidas);
        canetas_compradas_local = canetas_compradas;
        canetas_compradas = -1;
        pthread_mutex_unlock(&mutex_informacao_canetas_transferidas);
        
        total += canetas_compradas_local;
        
        printf("Comprou %d canetas\n", canetas_compradas_local);
        printf("Total comprado: %d\n", total);
    }

    // join em todas as threads
    pthread_join(thread_deposito_materia, 0);
    printf("Thread deposito materia finalizada");
    pthread_join(thread_fabrica, 0);
    printf("Thread fabrica finalizada");
    pthread_join(thread_controle, 0);
    printf("Thread controle finalizada");
    pthread_join(thread_deposito_caneta, 0);
    printf("Thread deposito caneta finalizada");
    pthread_join(thread_comprador, 0);
    printf("Thread comprador finalizada");

    // destroi mutexes
    pthread_mutex_destroy(&mutex_materia_enviada);
    pthread_mutex_destroy(&mutex_caneta_fabricar);
    pthread_mutex_destroy(&mutex_espaco_deposito);
    pthread_mutex_destroy(&mutex_caneta_transferida_deposito);
    pthread_mutex_destroy(&mutex_canetas_solicitadas);
    pthread_mutex_destroy(&mutex_canetas_transferidas_comprador);
    pthread_mutex_destroy(&mutex_informacao_canetas_transferidas);
    
    // destroi variaveis condicionais
    pthread_cond_destroy(&cond_caneta_transferida_deposito);
    pthread_cond_destroy(&cond_canetas_solicitadas);
    pthread_cond_destroy(&cond_canetas_transferidas_comprador);
    pthread_cond_destroy(&cond_informacao_canetas_transferidas);

    return 0;
}


void *deposito_materia() {
    // recebe quantidade de canetas para transferir pelo controle e manda para a fabrica (2 variaveis)
    printf("Thread deposito materia inicializada\n");

    int materia_prima_local = materia_existente;
    int demanda_caneta_local = 0;
    int materia_enviada_local;

    while(materia_prima_local > 0) {
        // obtem a demanda de canetas de controle
        pthread_mutex_lock(&mutex_caneta_fabricar);
        demanda_caneta_local = demanda_caneta;
        pthread_mutex_unlock(&mutex_caneta_fabricar);

        // seleciona quantidade de materia prima a enviar
        if(materia_prima_local >= demanda_caneta_local)
            materia_enviada_local = demanda_caneta_local;
        else
            materia_enviada_local = materia_prima_local;
        materia_prima_local -= materia_enviada_local;

        // envia materia prima para a fabrica
        pthread_mutex_lock(&mutex_materia_enviada);
        while(materia_enviada > 0)
            pthread_cond_wait(&cond_materia_recebida, &mutex_materia_enviada);
        materia_enviada = materia_enviada_local;
        pthread_mutex_unlock(&mutex_materia_enviada);

        // aguarda o intervalo entre envios
        sleep(intervalo_envio_materia);
    }
}

void *fabrica() {
    // recebe quantidade de canetas para produzir de controle, materia do deposito e manda para o outro deposito (3 variaveis)
    printf("Thread fabrica inicializada\n");
    int materia_prima_local = 0;
    int demanda_caneta_local = 0;

    while(1) {
        // obtem a demanda de canetas de controle
        pthread_mutex_lock(&mutex_caneta_fabricar);
        demanda_caneta_local = demanda_caneta;
        pthread_mutex_unlock(&mutex_caneta_fabricar);

        // recebe materia prima do deposito de materia prima
        pthread_mutex_lock(&mutex_materia_enviada);
        materia_prima_local += materia_enviada;
        materia_enviada = 0;
        pthread_cond_signal(&cond_materia_recebida);
        pthread_mutex_unlock(&mutex_materia_enviada);

        // fabrica caneta
        if(demanda_caneta_local > 0 && materia_prima_local > 0) {
            materia_prima_local--;

            // aguarda o tempo de fabricacao
            sleep(tempo_fabricacao);

            // transfere uma caneta ao deposito
            pthread_mutex_lock(&mutex_caneta_transferida_deposito);
            while(caneta_transferida_deposito == 1) // aguarda deposito receber caneta anterior
                pthread_cond_wait(&cond_caneta_transferida_deposito, &mutex_caneta_transferida_deposito);
            caneta_transferida_deposito = 1;
            pthread_mutex_unlock(&mutex_caneta_transferida_deposito);
        }
    }
}

void *controle() {
    // recebe quantidade de espacos vazios do deposito de canetas e solicita producao para a fabrica e deposito de materia (2 variaveis) : 
    printf("Thread controle inicializada\n");
    int espaco_deposito_local = 0;

    while(1) {
        // recebe espacos vazios do deposito de canetas
        pthread_mutex_lock(&mutex_espaco_deposito);
        espaco_deposito_local = espaco_deposito;
        pthread_mutex_unlock(&mutex_espaco_deposito);

        // informa producao desejada ao deposito de materiais e a fabrica
        pthread_mutex_lock(&mutex_caneta_fabricar);
        demanda_caneta = espaco_deposito_local;
        pthread_mutex_unlock(&mutex_caneta_fabricar);
    }
}

void *deposito_caneta() {
    // recebe pedidos de compra do comprador e transfere para ele as canetas compradas (2 variaveis) : canetas_solicitadas, canetas_transferidas_comprador
    // recebe caneta da fabrica e informa espacos vazios ao controle (2 variaveis) : caneta_transferida_deposito, espaco_deposito
    printf("Thread deposito caneta inicializada\n");

    int canetas_armazenadas = 0;
    int canetas_solicitadas_local = 0;
    int canetas_transferidas_local = 0;

    while(1) {
        // passa espacos vazios ao controle
        pthread_mutex_lock(&mutex_espaco_deposito);
        espaco_deposito = maximo_canetas_armazendas - canetas_armazenadas;
        pthread_mutex_unlock(&mutex_espaco_deposito);

        // recebe caneta da fabrica
        pthread_mutex_lock(&mutex_caneta_transferida_deposito);
        canetas_armazenadas += caneta_transferida_deposito;
        caneta_transferida_deposito = 0;
        pthread_cond_signal(&cond_caneta_transferida_deposito); // sinaliza recebimento da caneta
        pthread_mutex_unlock(&mutex_caneta_transferida_deposito);

        // parte do comprador
        // recebe novos pedidos do comprador apenas se o ultimo foi atendido
        if(canetas_solicitadas_local == 0) {
            // verifica se canetas foram solicitadas
            pthread_mutex_lock(&mutex_canetas_solicitadas);
            canetas_solicitadas_local = canetas_solicitadas;
            canetas_solicitadas = 0;
            pthread_mutex_unlock(&mutex_canetas_solicitadas);
        }

        // envia canetas ao comprador
        if(canetas_solicitadas_local != 0) {
            // envia quantidade solicitada de canetas
            pthread_mutex_lock(&mutex_canetas_transferidas_comprador);
            if(canetas_solicitadas_local <= canetas_armazenadas){
                canetas_transferidas_comprador = canetas_solicitadas_local;
            }
            else{
                canetas_transferidas_comprador = canetas_armazenadas;
            }

            canetas_transferidas_local = canetas_transferidas_comprador;
            pthread_cond_signal(&cond_canetas_transferidas_comprador);
            pthread_mutex_unlock(&mutex_canetas_transferidas_comprador);
            
            // atualizacao de variaveis fora da regiao critica
            canetas_armazenadas -= canetas_transferidas_local;
            canetas_solicitadas_local = 0;
        }
    }


}

void *comprador() {
    // solicita compra de canetas e recebe canetas compradas do deposito (2 variaveis) : canetas_solicitadas, canetas_transferidas_comprador
    // informa ao criador da compra (1 variavel) : canetas_compradas
    printf("Thread comprador inicializada\n");
    int canetas_compradas_local;

    while(1) {
        // aguarda o tempo entre compras
        sleep(tempo_compra);
        
        // solicita canetas
        pthread_mutex_lock(&mutex_canetas_solicitadas);
        canetas_solicitadas = canetas_por_compra;
        pthread_mutex_unlock(&mutex_canetas_solicitadas);

        // as recebe
        pthread_mutex_lock(&mutex_canetas_transferidas_comprador);
        pthread_cond_wait(&cond_canetas_transferidas_comprador, &mutex_canetas_transferidas_comprador);
        canetas_compradas_local = canetas_transferidas_comprador;
        canetas_transferidas_comprador = 0;
        pthread_mutex_unlock(&mutex_canetas_transferidas_comprador);
        
        //'envia' informação de compra para o criador
        pthread_mutex_lock(&mutex_informacao_canetas_transferidas);
        canetas_compradas = canetas_compradas_local;
        pthread_cond_signal(&cond_informacao_canetas_transferidas);
        pthread_mutex_unlock(&mutex_informacao_canetas_transferidas);
    }

}
