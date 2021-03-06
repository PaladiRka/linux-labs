#include <iostream>
#include <fstream>

#include <sys/sem.h>

#include "shmem_helpers.h"
#include "sem_helpers.h"

constexpr unsigned BUFFER_LENGTH {3};

constexpr key_t BUFFER_SHMEM_KEY {0x8d};
constexpr key_t BUFFER_LENGTH_SHMEM_KEY {0x8e};
constexpr key_t THERE_WILL_BE_NO_MORE_DATA_SHMEM_KEY {0x8f};

constexpr key_t SEM_KEY {0x90};
constexpr short EMPTY_COUNT_SEM_NUM {0};
constexpr short FILL_COUNT_SEM_NUM {1};
constexpr short BUFFER_RW_SEM_NUM {2};
constexpr short END_SUM_NUM {3};

int semaphore_id {};

int main(int argc, char** argv) {
    std::cout << "Consumer" << std::endl;

    auto [buffer_length_shmem_id, buffer_length_ptr] = allocate_shmem_segment<int>(BUFFER_LENGTH_SHMEM_KEY);
    *buffer_length_ptr = BUFFER_LENGTH;

    auto [circular_buffer_shmem_id, circular_buffer] = allocate_shmem_segment<char>(BUFFER_SHMEM_KEY, BUFFER_LENGTH);

    semaphore_id = semget(SEM_KEY, 4, 0662 | IPC_CREAT);

    char character;
    int char_index {};
    while (true) {
        decrement_simple_semaphore(semaphore_id, FILL_COUNT_SEM_NUM);
        decrement_simple_semaphore(semaphore_id, BUFFER_RW_SEM_NUM);
        character = circular_buffer[char_index];
        std::cout << character << std::flush;
        increment_simple_semaphore(semaphore_id, BUFFER_RW_SEM_NUM);
        increment_simple_semaphore(semaphore_id, EMPTY_COUNT_SEM_NUM);

        char_index = (char_index + 1) % BUFFER_LENGTH;

        if (character == '\0') {
            std::cout << std::endl;
            break;
        }
    }
    decrement_simple_semaphore(semaphore_id, END_SUM_NUM);
    shmdt(circular_buffer);
    shmdt(buffer_length_ptr);
    shmctl(buffer_length_shmem_id, IPC_RMID, nullptr);
    shmctl(circular_buffer_shmem_id, IPC_RMID, nullptr);

    semctl(semaphore_id, 3, IPC_RMID);

    return 0;
}
