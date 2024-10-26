/*
 * mandel.c
 *
 * A program to draw the Mandelbrot Set
 on a 256-color xterm.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>
#include "mandel-lib.h"
#include <semaphore.h>
#include <errno.h>
#include <stdint.h>
#define perror_pthread(ret, msg) \
        do { errno = ret; perror(msg); } while (0)


#define MANDEL_MAX_ITERATION 100000

/***************************
 * Compile-time parameters *
 ***************************/

/*
 * Output at the terminal is is x_chars
 wide by y_chars long
 */
int *fst;
int y_chars = 50;
int x_chars = 90;
uint64_t va;
/*
 * The part of the complex plane to be drawn:
 * upper left corner is (xmin, ymax), 
 lower right corner is (xmax, ymin)
 */
double xmin = -1.8, xmax = 1.0;
double ymin = -1.0, ymax = 1.0;

/*
 * Every character in the final output is
 * xstep x ystep units wide on the complex plane.
 */
double xstep;
double ystep;

struct pair{
        int thrcount;
        int N;
};

/*
 * This function computes a line of output
 * as an array of x_char color values.
 */
void compute_mandel_line(int line, int color_val[])
{
        /*
         * x and y traverse the complex plane.
         */
        double x, y;

        int n;
        int val;

        /* Find out the y value corresponding to this line */
        y = ymax - ystep * line;

        /* and iterate for all points on this line */
        for (x = xmin, n = 0; n < x_chars; x+= xstep, n++) {

                /* Compute the point's color value */
                val = mandel_iterations_at_point(x, y,
                MANDEL_MAX_ITERATION);
                if (val > 255)
                        val = 255;

                /* And store it in the color_val[] array */
                val = xterm_color(val);
                color_val[n] = val;

        }
}

/*
 * This function outputs an array of x_char color values
 * to a 256-color xterm.
 */
void output_mandel_line(int fd, int color_val[])
{
        int i;

        char point ='@';
        char newline='\n';

        for (i = 0; i < x_chars; i++) {
                /* Set the current color,
                then output the point */
                set_xterm_color(fd, color_val[i]);

                if (write(fd, &point, 1) != 1){
                        perror("compute_and_output_mandel_line:
                        write point");
                        exit(1);
                }


        }
        /* Now that the line is done, output 
        a newline character */
        if (write(fd, &newline, 1) != 1) {
                perror("compute_and_output_mandel_line:
                write newline");
                exit(1);
        }
}

void* compute_and_output_mandel_line(void *arg)
{
        volatile struct pair *pair = arg;
        int thrcount = pair->thrcount;
        int N = pair->N;
        int i,j;
        for(i=thrcount;i<y_chars;i+=N){

                /*
                * A temporary array, used to hold 
                color values for the line being drawn
                 */

                int color_val[x_chars];


                compute_mandel_line(i, color_val);
                for(j=0;j<x_chars;j++)
                        *(fst + i*x_chars + j)=color_val[j];

        }
        exit(12);
}

int main(int argc, char *argv[])
{
        int i,N;
        int fd;
        int ret;
        int status;
        N=atoi(argv[1]);

        struct pair pair[N];

        pid_t pr[i];

        va=mmap(NULL, x_chars*y_chars*sizeof(int),
        PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
        fst = va;
        xstep = (xmax - xmin) / x_chars;
        ystep = (ymax - ymin) / y_chars;

        /*
         * draw the Mandelbrot Set, one line at a time.
         * Output is sent to file descriptor 
         '1', i.e., standard output.
         */
        for(i=0;i<N;i++){
                pair[i].thrcount=i;
                pair[i].N=N;
                pr[i]=fork();
                if (pr[i]==0) {

                         compute_and_output_mandel_line(&pair[i]);
                }

        }

        for(i=0; i<N; i++)
                pr[i]=wait(&status);

        fd=1;

        for(i=0; i<y_chars;i++)
                output_mandel_line(fd,fst + i*x_chars);

        reset_xterm_color(1);
        return 0;
}
