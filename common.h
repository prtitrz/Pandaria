/*
 * not use coding, decomposition and recombine now.
 */
struct setting {
	int records_num;
	int threads_num;
	int pool_size; 
	char coding[10];
	char decomposition[20];
	char recombine[20];
	char spc_loc[257];
	char location[257];
};

struct record {
	int dev_num;
	long long offset;
	long length;
	char op;
	struct timeval time;
	struct timeval res_time;
};

int init_conf(struct setting *);

int init_rec_array(const struct setting, struct record *);
