#ifndef __PIPELINE_H
#define __PIPELINE_H

pipeline create_pipeline(int num_commands, int fg_or_bg);
void destroy_pipeline(pipeline p);
int compare_pipelines(pipeline *A, int num_pipelines_A, pipeline *B, int num_pipelines_B);

void handlePipeline(pipeline P);

#endif