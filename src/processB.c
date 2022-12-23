#include <bmpfile.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "./../include/processB_utilities.h"
#include "./../include/circle_utilities.h"
#define SEM_PATH_WRITER "/sem_AOS_writer"
#define SEM_PATH_READER "/sem_AOS_reader"

int main(int argc, char const *argv[])
{
    // instantiation of the shared memory
    const char * shm_name = "/AOS";
    const int SIZE = width*height*sizeof(rgb_pixel_t);
    int shm_fd;
    rgb_pixel_t * ptr;
    sem_t *sem_id_writer;
    sem_t *sem_id_reader;

    shm_fd = shm_open(shm_name, O_RDONLY, 0666);
    if (shm_fd == 1) {
        printf("Shared memory segment failed\n");
        exit(1);
    }

    ptr = (rgb_pixel_t *)mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        printf("Map failed\n");
        return 1;
    }

    // DA GESTIRE IN VERSIONE FINALE, QUANDO PROGRAMMA CHIUDE
    //munmap(ptr, SIZE);


    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // normalized center for the ncurse window
    int cx, cy, counter = 0, dim = 10;

    // array for the center
    coordinate *center = NULL;

    center = (coordinate*) malloc(dim * sizeof (coordinate));


    // Initialize UI
    init_console_ui();

    // Data structure for storing the bitmap file
    bmpfile_t *bmp;
    
    bmp = bmp_create(width, height, depth);

    //circle_draw(center[0].x, center[0].y,bmp);

    sem_id_writer = sem_open(SEM_PATH_WRITER, 0);
    if(sem_id_writer== (void*)-1){
        perror("sem_open failure");
        exit(1);
    }

    sem_id_reader = sem_open(SEM_PATH_READER, 0);
    if(sem_id_reader== (void*)-1){
        perror("sem_open failure");
        exit(1);
    }
    
    // Infinite loop
    while (TRUE) {

        // Get input in non-blocking mode
        int cmd = getch();

        // If user resizes screen, re-draw UI...
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }
        else {
            sem_wait(sem_id_reader);
            center[counter] = find_center(bmp, ptr);
            sem_post(sem_id_writer);
            //circle_draw(center[0].x, center[0].y,bmp);
            cx = center[counter].x/20;
            cy = center[counter].y/20;
            mvaddch(cy, cx, '0');
            delete(center[counter].x, center[counter].y,bmp);
            if(counter == dim){
                dim = 2*dim;
                center = (coordinate*) realloc(center, dim* sizeof (coordinate));
            }
            counter++;
            refresh();
        }
    }

    endwin();
    return 0;
}
