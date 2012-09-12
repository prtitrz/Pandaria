#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "debug.h"

int init_conf(struct setting *setting) 
{
	FILE *fp;
	char buf[1024];
	int i=0;

	fp = fopen("./pandaria.conf", "r");
	if (fp == NULL) {
		debug_puts("FILE can't open");
		return 1;
	}

	while (fgets(buf, 1024, fp) != NULL) {
		if (sscanf(buf, "spc_loc=%s", setting->spc_loc) == 1) {
			continue;
		}
		if (sscanf(buf, "loc=%s", setting->location) == 1) {
			continue;
		}
		if (sscanf(buf, "records_num=%ld", &setting->records_num) == 1) {
			continue;
		}
		if (sscanf(buf, "threads_num=%d", &setting->threads_num) == 1) {
			continue;
		}
		if (sscanf(buf, "pool_size=%d", &setting->pool_size) == 1) {
			continue;
		}
	}

	fclose(fp);

	return 0;
}

int init_rec_array(const struct setting setting, struct record *records)
{
	FILE *fp;
	int i;
	double time;
	char tmp[20];

	fp = fopen(setting.spc_loc, "r");
	if (fp == NULL) {
		debug_puts("RECORD FILE OPEN ERROR");
		return 1;
	}

	for (i = 0; i < setting.records_num; i++) {
		fscanf(fp, "%d,%lld,%ld,%c,%lf", &(records[i]).dev_num, &records[i].offset, \
				&records[i].length, &records[i].op, &time);
		sprintf(tmp, "%lf", time);
		sscanf(tmp, "%ld.%ld", &records[i].time.tv_sec, \
				&records[i].time.tv_usec);
		/*
		 * According the doc, the meaning of offset is LBA. So..
		 */
		records[i].offset = records[i].offset * 512;
	}
	
	fclose(fp);
	return 0;
}

