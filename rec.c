#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <sys/time.h>

#define MAX_FRIENDS 50
#define MAX_CANDIDATES 200

typedef struct {
    int id;
    int common;   // количество общих знакомых
} Candidate;

typedef struct {
    int *friends;
    int count;
} User;

// Генерация синтетического графа
void generate_graph(User *users, int N) {
    for (int i = 0; i < N; i++) {
        users[i].count = 5 + rand() % 20;   // от 5 до 24 друзей
        users[i].friends = (int*)malloc(users[i].count * sizeof(int));
        
        for (int j = 0; j < users[i].count; j++) {
            int f;
            do {
                f = rand() % N;
            } while (f == i);
            users[i].friends[j] = f;
        }
    }
}

// Проверка, являются ли a и b друзьями
int is_friend(User *users, int a, int b) {
    for (int i = 0; i < users[a].count; i++) {
        if (users[a].friends[i] == b) return 1;
    }
    return 0;
}

// Подсчёт общих знакомых
int count_common(User *users, int a, int b) {
    int common = 0;
    for (int i = 0; i < users[a].count; i++) {
        int f = users[a].friends[i];
        if (is_friend(users, b, f)) common++;
    }
    return common;
}

// Рекомендации для одного пользователя
int recommend_for_user(User *users, int user_id, int N, Candidate *result) {
    int cand_count = 0;
    int *visited = (int*)calloc(N, sizeof(int));
    visited[user_id] = 1;

    // Помечаем уже существующих друзей
    for (int i = 0; i < users[user_id].count; i++) {
        visited[users[user_id].friends[i]] = 1;
    }

    // Смотрим друзей друзей
    for (int i = 0; i < users[user_id].count; i++) {
        int friend_id = users[user_id].friends[i];
        for (int j = 0; j < users[friend_id].count; j++) {
            int candidate = users[friend_id].friends[j];
            if (!visited[candidate]) {
                visited[candidate] = 1;
                int common = count_common(users, user_id, candidate);
                if (common > 0 && cand_count < MAX_CANDIDATES) {
                    result[cand_count].id = candidate;
                    result[cand_count].common = common;
                    cand_count++;
                }
            }
        }
    }

    // Сортировка по убыванию общих знакомых (простая)
    for (int i = 0; i < cand_count - 1; i++) {
        for (int j = i + 1; j < cand_count; j++) {
            if (result[j].common > result[i].common) {
                Candidate tmp = result[i];
                result[i] = result[j];
                result[j] = tmp;
            }
        }
    }

    free(visited);
    return cand_count;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s N [num_threads]\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int num_threads = 1;
    if (argc >= 3) num_threads = atoi(argv[2]);

    omp_set_num_threads(num_threads);

    printf("Generating graph with %d users...\n", N);
    User *users = (User*)malloc(N * sizeof(User));
    srand(42);
    generate_graph(users, N);

    // Массив для хранения количества рекомендаций (чтобы компилятор не выкинул вычисления)
    int *rec_counts = (int*)malloc(N * sizeof(int));

    double T1 = omp_get_wtime();

    #pragma omp parallel for schedule(dynamic, 16)
    for (int i = 0; i < N; i++) {
        Candidate candidates[MAX_CANDIDATES];
        rec_counts[i] = recommend_for_user(users, i, N, candidates);
    }

    double T2 = omp_get_wtime();
    double time_ms = (T2 - T1) * 1000.0;

    printf("N=%d threads=%d. Time: %.1f ms\n", N, num_threads, time_ms);

    // Очистка
    for (int i = 0; i < N; i++) free(users[i].friends);
    free(users);
    free(rec_counts);

    return 0;
}
