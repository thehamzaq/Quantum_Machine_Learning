#ifndef PHASE_LOSS_H
#define PHASE_LOSS_H

/*NOTE on using fitness functions for phase estimation problem with loss*/
/* Both avg_fitness() and fitness() contain the same code.
 * The policies are learned without loss (loss in avg_fitness() set to zero).
 * Then the policies are selected based on its mean fitness value for lossy
 * interferometer (loss in fitness() set to other than zero) which is called
 * through avg_Final_select() in OptAlg class.
 * */

#define DEV_N 0.0 //level of noise in operator
#define THETA_DEV 0.0 //M_PI;//phase noise level

#include "problem.h"

class Phase: public Problem<double>
{
public:
    Phase(const int numvar);
    ~Phase();

    double fitness(double *soln);
    double avg_fitness(double *soln, const int K);

private:
    double lower;
    double upper;
    //array to avoid calculation of expensive sqrt calls for integers
    double *sqrt_cache;
    //infrastructure to store random numbers that are generated vectorized
    double *urandom_numbers;
    int n_urandom_numbers;
    int index_urandom_numbers;
    void init_urandom_number_cache(const int n);
    inline double next_urand();
    double *grandom_numbers;
    int n_grandom_numbers;
    int index_grandom_numbers;
    void init_grandom_number_cache(const int n);
    inline double next_grand(const double mean, const double dev);
    void status_rand();

    //variables for WK state generation
    dcmplx *input_state;
    double *sqrtfac_mat; //matrix to keep values of square roots of factorials
    double *overfac_mat; //matrix to keep values of one over factorials
    double tan_beta;
    //state for use with measurement
    dcmplx *state;
    //variables for noise_output function
    dcmplx *update0;
    dcmplx *update1;

    //functions to generate WK_state
    inline void sqrtfac(double *fac_mat);
    inline void one_over_fac(double *over_mat);
    inline double cal_spart(const int n, const int k, const int N);//N is is the same as total number of photon 'num', but I'll leave it like this.
    void WK_state();
    //Measurement function
    inline bool outcome(const double phi, const double PHI, const int N);//N is the number of photons currently available, not equal to 'num'
    inline bool noise_outcome(const double phi, const double PHI, const int N);
    inline void state_loss(const int N);
    inline double rand_Gaussian(const double mean, const double dev);
    inline double mod_2PI(double PHI);
};
#endif // PHASE_H