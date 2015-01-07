#ifndef PSO_H
#define PSO_H

#include "mpi_optalg.h"

template <typename typeT>
class PSO : public OptAlg<typeT> {
public:
    PSO() {};
    PSO(Problem<typeT> *problem_ptr):w(0.8),phi1(0.6),phi2(1.0),v_max(0.2) {
        this->prob=problem_ptr;
        this->num=this->prob->num;
    }
    ~PSO() {};

    void put_to_best(int my_rank, int total_pop, int nb_proc);
    void combination(int my_rank, int total_pop, int nb_proc);
    void selection(int my_rank, int total_pop, int nb_proc);
    void write_param(double *param_array);
    void fit_to_global() {};
    void find_global(int my_rank, int total_pop, int nb_proc);

private:
    double w,phi1,phi2,v_max;
};

template <typename typeT>
void PSO<typeT>::write_param(double *param_array) {
    w=param_array[1];
    phi1=param_array[2];
    phi2=param_array[3];
    v_max=param_array[4];
}

template <typename typeT>
void PSO<typeT>::put_to_best(int my_rank, int total_pop, int nb_proc) {
    int i,p;
    typeT array[this->num];

    for(p=0; p<this->pop_size; ++p) {
        this->pop[p].update_best();
        this->pop[p].init_velocity();
        for(i=0; i<this->num; ++i) {
            array[i]=double(rand())/RAND_MAX*v_max;
        }
        this->pop[p].update_vel(array);
    }

    find_global(my_rank,total_pop,nb_proc);

}

template <typename typeT>
void PSO<typeT>::find_global(int my_rank, int total_pop, int nb_proc) {
    MPI_Status status;
    MPI_Datatype MPI_TYPE;
    int tag=1;
    int p,ptr,prev,forw;
    double prev_fit,forw_fit,fit;
    typeT array[this->num];

    if(typeid(array[0])==typeid(double)) {
        MPI_TYPE=MPI_DOUBLE;
    }
    else if(typeid(array[0])==typeid(dcmplx)) {
        MPI_TYPE=MPI_DOUBLE_COMPLEX;
    }
    else {}

    for(p=0; p<total_pop; ++p) {

        //find index of the candidate in the neighborhood
        if(p==0) {
            prev=total_pop-1;
        }
        else {
            prev=p-1;
        }
        forw=(p+1)%total_pop;

        if(my_rank==prev%nb_proc) {
            prev_fit=this->pop[prev/nb_proc].read_bestfit();
            MPI_Send(&prev_fit,1,MPI_DOUBLE,p%nb_proc,tag,MPI_COMM_WORLD);
        }
        else if(my_rank==p%nb_proc) {
            MPI_Recv(&prev_fit,1,MPI_DOUBLE,prev%nb_proc,tag,MPI_COMM_WORLD,&status);
        }
        else {}

        if(my_rank==forw%nb_proc) {
            forw_fit=this->pop[forw/nb_proc].read_bestfit();
            MPI_Send(&forw_fit,1,MPI_DOUBLE,p%nb_proc,tag,MPI_COMM_WORLD);
        }
        else if(my_rank==p%nb_proc) {
            MPI_Recv(&forw_fit,1,MPI_DOUBLE,forw%nb_proc,tag,MPI_COMM_WORLD,&status);
        }
        else {}

        MPI_Barrier(MPI_COMM_WORLD);

        //fitness values sent to p
        if(my_rank==p%nb_proc) {
            fit=this->pop[p/nb_proc].read_bestfit();//read fitness of p
            ptr=prev;
            if(prev_fit<=fit) {
                ptr=p;
                if(fit<forw_fit) {
                    ptr=forw;
                }
                else {} //ptr=p
            }
            else if(prev_fit>fit) {
                //ptr=prev
                if(prev_fit<forw_fit) {
                    ptr=forw;
                }
                else {} //ptr still points to prev
            }
            else {
                cout<<"Error!!!"<<endl;
            }
        }

        //send ptr
        if(my_rank==p%nb_proc) {
            MPI_Send(&ptr,1,MPI_INT,prev%nb_proc,tag,MPI_COMM_WORLD);
        }
        else if(my_rank==prev%nb_proc) {
            MPI_Recv(&ptr,1,MPI_INT,p%nb_proc,tag,MPI_COMM_WORLD,&status);
            //cout<<",prev ptr="<<ptr;
        }
        else {}



        if(my_rank==p%nb_proc) {
            MPI_Send(&ptr,1,MPI_INT,forw%nb_proc,tag,MPI_COMM_WORLD);
        }
        else if(my_rank==forw%nb_proc) {
            MPI_Recv(&ptr,1,MPI_INT,p%nb_proc,tag,MPI_COMM_WORLD,&status);
            //cout<<",forw ptr="<<ptr<<endl;
        }
        else {}

        MPI_Barrier(MPI_COMM_WORLD);

        //updating global best
        if(ptr==p) { //Global best already in precessor's memory. No need for MPI
            if(my_rank==ptr%nb_proc) {
                this->pop[p/nb_proc].read_best(array);
                this->pop[p/nb_proc].update_global(array);
                this->pop[p/nb_proc].write_globalfit(fit);
            }
            else {}
            //cout<<"ptr=p:"<<ptr<<","<<p<<endl;
        }
        else if(ptr==prev||ptr==forw) {
            //cout<<"ptr:"<<ptr<<",prev="<<prev<<",forw="<<forw<<endl;
            if(my_rank==ptr%nb_proc) {
                this->pop[ptr/nb_proc].read_best(array);
                MPI_Send(&array,this->num,MPI_TYPE,p%nb_proc,tag,MPI_COMM_WORLD);
            }
            else if(my_rank==p%nb_proc) {
                MPI_Recv(&array,this->num,MPI_TYPE,ptr%nb_proc,tag,MPI_COMM_WORLD,&status);
                this->pop[p/nb_proc].update_global(array);
                if(ptr==prev) {
                    this->pop[p/nb_proc].write_globalfit(prev_fit);
                }
                else if(ptr==forw) {
                    this->pop[p/nb_proc].write_globalfit(forw_fit);
                }
                else {
                    cout<<"Error!! ptr not pointing correctly"<<endl;
                }
            }
        }
        else {
            //cout<<"ptr:"<<ptr<<",prev="<<prev<<",forw="<<forw<<endl;
            //cout<<"Can't find global. Code Error"<<endl;
        }
        //end updating global best

    }//p loop
}

template <typename typeT>
void PSO<typeT>::combination(int my_rank, int total_pop, int nb_proc) {
    int p,i;
    typeT global_pos[this->num];
    typeT personal_pos[this->num];
    typeT pos[this->num];
    typeT vel[this->num];
    typeT new_pos[this->num];

    for(p=0; p<this->pop_size; ++p) {
        srand(time(NULL)+p);
        i=rand();//flushing out the deterministic results
        this->pop[p].read_global(global_pos);
        this->pop[p].read_best(personal_pos);
        this->pop[p].read_cont(pos);
        this->pop[p].read_vel(vel);
        for(i=0; i<this->num; ++i) {
            new_pos[i]=pos[i]+vel[i];
            vel[i]=vel[i]+phi1*double(rand())/RAND_MAX*(personal_pos[i]-pos[i])+phi2*double(rand())/RAND_MAX*(global_pos[i]-pos[i]);
            if(vel[i]>v_max) {
                vel[i]=v_max;
            }
            else if(vel[i]<-v_max) {
                vel[i]=-v_max;
            }
            else {
                /*keep vel[i]*/
            }
        }
        //this->prob->normalize(new_pos);
        this->prob->modulo(new_pos);
        this->pop[p].update_cont(new_pos);
        this->pop[p].update_vel(vel);
        this->Cont_fitness(p);
    }
}

template <typename typeT>
void PSO<typeT>::selection(int my_rank, int total_pop, int nb_proc) {
    int p;
    for(p=0; p<this->pop_size; ++p) {
        if(this->pop[p].read_bestfit()<this->pop[p].read_contfit()) {
            this->pop[p].update_best();
        }
        else {
            /*do nothing cout<<endl;*/
        }
    }
    find_global(my_rank,total_pop,nb_proc);

}
#endif