#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N_USERS 100000
#define MAX_FRIENDS 30
#define TOP_K 10

typedef struct {
    int *friends;
    int count;
} User;

User *users;

void generate_graph() {
    users = (User*)malloc(N_USERS * sizeof(User));
    unsigned int seed = 42;

    for (int i = 0; i < N_USERS; i++) {
        int cnt = 6 + rand_r(&seed) % 20;
        users[i].count = cnt;
        users[i].friends = (int*)malloc(cnt * sizeof(int));
        for (int j = 0; j < cnt; j++) {
            int f;
            do { f = rand_r(&seed) % N_USERS; } while (f == i);
            users[i].friends[j] = f;
        }
    }
}

// Упрощённые рекомендации: просто собираем уникальных друзей друзей
int recommend(int user_id, int *result) {
    char *visited = (char*)calloc(N_USERS, 1);
    visited[user_id] = 1;

    // Помечаем своих друзей
    for (int i = 0; i < users[user_id].count; i++) {
        visited[users[user_id].friends[i]] = 1;
    }

    int count = 0;
    for (int i = 0; i < users[user_id].count && count < TOP_K; i++) {
        int fr = users[user_id].friends[i];
        for (int j = 0; j < users[fr].count && count < TOP_K; j++) {
            int cand = users[fr].friends[j];
            if (!visited[cand]) {
                visited[cand] = 1;
                result[count++] = cand;
            }
        }
    }

    free(visited);
    return count;
}

int main(int argc, char *argv[]) {
    int num_threads = 1;
    if (argc >= 2) num_threads = atoi(argv[1]);
    omp_set_num_threads(num_threads);

    printf("Generating graph (%d users)...\n", N_USERS);
    generate_graph();
    printf("Graph ready. Starting recommendations...\n");

    int *rec_counts = (int*)malloc(N_USERS * sizeof(int));

    double t1 = omp_get_wtime();

    #pragma omp parallel for schedule(dynamic, 128)
    for (int i = 0; i < N_USERS; i++) {
        int top[TOP_K];
        rec_counts[i] = recommend(i, top);
    }

    double t2 = omp_get_wtime();
    printf("Threads: %d | Time: %.3f sec\n", num_threads, t2 - t1);

    for (int i = 0; i < N_USERS; i++) free(users[i].friends);
    free(users);
    free(rec_counts);
    return 0;
}
