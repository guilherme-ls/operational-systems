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
 * digite as variaveis inteiras separadas por espaco na ordem da especificacao apos o ./main, antes de dar enter
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
int canetas_transferidas_comprador = -1;
int canetas_compradas = -1;

// Variaveis condicionais necessarias
pthread_cond_t cond_materia_recebida;
pthread_cond_t cond_caneta_transferida_deposito;
pthread_cond_t cond_canetas_transferidas_comprador;
pthread_cond_t cond_informacao_canetas_transferidas;
pthread_cond_t cond_recebimento_informacao_canetas_transferidas;

// Mutexes necessarios
pthread_mutex_t mutex_materia_enviada;
pthread_mutex_t mutex_caneta_fabricar;
pthread_mutex_t mutex_espaco_deposito;
pthread_mutex_t mutex_caneta_transferida_deposito;
pthread_mutex_t mutex_canetas_transferidas_comprador;
pthread_mutex_t mutex_canetas_solicitadas;
pthread_mutex_t mutex_informacao_canetas_transferidas;

// Thread criador (rank 0)
int main(int argc, char **argv){
    // checa validade dos argumentos recebidos
    if(argc != 7 + 1){
        printf("esperava 7 argumentos!\n");
        exit(1);
    }

    // separa argumentos recebidos
    sscanf(argv[1], "%d", &materia_existente);
    sscanf(argv[2], "%d", &materia_por_envio);
    sscanf(argv[3], "%d", &intervalo_envio_materia);
    sscanf(argv[4], "%d", &tempo_fabricacao);
    sscanf(argv[5], "%d", &maximo_canetas_armazendas);
    sscanf(argv[6], "%d", &canetas_por_compra);
    sscanf(argv[7], "%d", &tempo_compra);

    // chama a funcao criador para inicializar threads
    return criador();
}

/**
 * @brief Thread criador: recebe argumentos de entrada e instancia demais threads do programa
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
    pthread_cond_init(&cond_materia_recebida, NULL);
    pthread_cond_init(&cond_caneta_transferida_deposito, NULL);
    pthread_cond_init(&cond_canetas_transferidas_comprador, NULL);
    pthread_cond_init(&cond_informacao_canetas_transferidas, NULL);
    pthread_cond_init(&cond_recebimento_informacao_canetas_transferidas, NULL);

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

    // Loop para output das canetas compradas
    int canetas_compradas_local = 0;
    int total = 0;
    while(total < materia_existente){
        // receber informacao sobre canetas compradas
        pthread_mutex_lock(&mutex_informacao_canetas_transferidas);
        while(canetas_compradas == -1) // aguarda ter uma nova compra de canetas
            pthread_cond_wait(&cond_informacao_canetas_transferidas, &mutex_informacao_canetas_transferidas);
        canetas_compradas_local = canetas_compradas;
        canetas_compradas = -1;
        pthread_cond_signal(&cond_recebimento_informacao_canetas_transferidas); // sinaliza recebimento da informacao
        pthread_mutex_unlock(&mutex_informacao_canetas_transferidas);
        
        total += canetas_compradas_local;
        
        // output das canetas compradas
        printf("Comprou %d canetas\n", canetas_compradas_local);
        printf("Total comprado: %d\n", total);
    }

    // fim da execucao do programa
    printf("Producao concluida com sucesso!\n");
    exit(0);
    return 0;
}

/**
 * @brief Thread deposito de materia prima: transfere materia prima para a fabrica conforme indicacao do controle
 */
void *deposito_materia() {
    int materia_prima_local = materia_existente;
    int demanda_caneta_local = 0;
    int materia_enviada_local;

    while(1) {
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
        while(materia_enviada > 0) // aguarda recebimento da materia prima anterior
            pthread_cond_wait(&cond_materia_recebida, &mutex_materia_enviada);
        materia_enviada = materia_enviada_local;
        pthread_mutex_unlock(&mutex_materia_enviada);

        // aguarda o intervalo entre envios
        sleep(intervalo_envio_materia);
    }
}


/**
 * @brief Thread fabrica: recebe materia prima, produz canetas e as transfere ao deposito de canetas
 */
void *fabrica() {
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
        pthread_cond_signal(&cond_materia_recebida); // sinaliza recebimento da materia prima
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

/**
 * @brief Thread deposito controle: recebe quantidade de espacos vazios do deposito de canetas e 
 * solicita producao para a fabrica e deposito de materia prima
 */
void *controle() {
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

/**
 * @brief Thread deposito de canetas: recebe pedidos de compra do comprador e transfere para ele as canetas compradas,
 * tambem recebe caneta da fabrica e informa espacos vazios ao controle
 */
void *deposito_caneta() {
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
            // determina quantidade que pode ser enviada
            if(canetas_solicitadas_local > canetas_armazenadas)
                canetas_solicitadas_local = canetas_armazenadas;

            // envia quantidade solicitada de canetas
            pthread_mutex_lock(&mutex_canetas_transferidas_comprador);
            canetas_transferidas_comprador = canetas_solicitadas_local;
            canetas_transferidas_local = canetas_transferidas_comprador;
            pthread_cond_signal(&cond_canetas_transferidas_comprador); // sinaliza envio das canetas
            pthread_mutex_unlock(&mutex_canetas_transferidas_comprador);
            
            // atualizacao de variaveis fora da regiao critica
            canetas_armazenadas -= canetas_transferidas_local;
            canetas_solicitadas_local = 0;
        }
    }


}

/**
 * @brief Thread comprador: solicita compra de canetas e recebe canetas compradas do deposito,
 * tambem informa ao criador da compra
 */
void *comprador() {
    int canetas_compradas_local;

    while(1) {
        // solicita canetas
        pthread_mutex_lock(&mutex_canetas_solicitadas);
        canetas_solicitadas = canetas_por_compra;
        pthread_mutex_unlock(&mutex_canetas_solicitadas);

        // recebe canetas
        pthread_mutex_lock(&mutex_canetas_transferidas_comprador);
        while(canetas_transferidas_comprador < 0) // espera ter canetas a receber
            pthread_cond_wait(&cond_canetas_transferidas_comprador, &mutex_canetas_transferidas_comprador);
        canetas_compradas_local = canetas_transferidas_comprador;
        canetas_transferidas_comprador = -1;
        pthread_mutex_unlock(&mutex_canetas_transferidas_comprador);
        
        //'envia' informação de compra para o criador
        pthread_mutex_lock(&mutex_informacao_canetas_transferidas);
        while(canetas_compradas > -1) // aguarda criador receber ultima informacao de compra
            pthread_cond_wait(&cond_recebimento_informacao_canetas_transferidas, &mutex_informacao_canetas_transferidas);
        canetas_compradas = canetas_compradas_local;
        pthread_cond_signal(&cond_informacao_canetas_transferidas); // informa que canetas foram compradas
        pthread_mutex_unlock(&mutex_informacao_canetas_transferidas);
        
        // aguarda o tempo entre compras
        sleep(tempo_compra);
    }

}
