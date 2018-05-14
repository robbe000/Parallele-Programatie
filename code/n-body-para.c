#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#ifdef __APPLE__
# include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif

#include "time_utils.h"
#include "ocl_utils.h"
#include "math.h"
#include "renderer.h"

#define WIDTH (640 * 10)
#define HEIGHT (480 * 10)

void usage(char* prog_name)
{
    printf("Usage: %s <number of bodies>\n", prog_name);
}


cl_mem create_result_buffer(int num_bodies)
{
    cl_int error;

    cl_mem dev_vec = clCreateBuffer(g_context, CL_MEM_WRITE_ONLY,
            sizeof(cl_float3) * num_bodies, NULL, &error);
    ocl_err(error);
    return dev_vec;
}

void simulate_gravity(cl_float3* host_pos, cl_float3* host_speed, int length, cl_mem dev_result, cl_mem crnl_pos, cl_mem crnl_speed)
{
	cl_int error;
    // Create device buffers.
    

	// Create kernel
    cl_kernel kernel = clCreateKernel(g_program, "simulate_gravity", &error);
    ocl_err(error);

	// Set kernel arguments
    ocl_err(error);
    ocl_err(error);
    int arg_num = 0;
    //ocl_err(clSetKernelArg(kernel, arg_num++, sizeof(cl_mem), &dev_result));
    ocl_err(clSetKernelArg(kernel, arg_num++, sizeof(cl_mem), &crnl_pos));
    ocl_err(clSetKernelArg(kernel, arg_num++, sizeof(cl_mem), &crnl_speed));
    ocl_err(clSetKernelArg(kernel, arg_num++, sizeof(cl_int), &length));
   	// Call kernel
    size_t global_work_sizes = length;
    //time_measure_start("computation");
    ocl_err(clEnqueueNDRangeKernel(g_command_queue, kernel, 1, NULL,
                &global_work_sizes, NULL, 0, NULL, NULL));
    ocl_err(clFinish(g_command_queue));
    //time_measure_stop_and_print("computation");

    // Read result
    // time_measure_start("data_transfer");
    ocl_err(clEnqueueReadBuffer(g_command_queue, crnl_pos, CL_TRUE,
                0, sizeof(cl_float3)*length, host_pos, 0, NULL, NULL));
    //time_measure_stop_and_print("data_transfer");
	//printf("%f %f %f", host_pos[0].s[0], host_pos[0].s[1], host_pos[0].s[2]);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        usage(argv[0]);
        return 1;
    }
    int num_bodies = atoi(argv[1]);
	cl_int error;

    init_gl();

    cl_float3 *host_pos = malloc(sizeof(cl_float3) * num_bodies);
    cl_float3 *host_speed = malloc(sizeof(cl_float3) * num_bodies);
    for (int i = 0; i < num_bodies; ++i)
    {
        float offset;

        if (rand() < RAND_MAX / 2)
            offset = -5.f;
        else
            offset = 5.f;


        host_pos[i].s[0] = ((float)rand() / (float)RAND_MAX) * 2.f - 1.f + offset;
        host_pos[i].s[1] = ((float)rand() / (float)RAND_MAX) * 2.f - 1.f;
        host_pos[i].s[2] = ((float)rand() / (float)RAND_MAX) * 2.f - 1.f;

        host_speed[i].s[0] = 0.f;
        host_speed[i].s[1] = 0.f;
        host_speed[i].s[2] = 0.f;
    }
	
   cl_platform_id platform = ocl_select_platform();
   cl_device_id device = ocl_select_device(platform);
   init_ocl(device);
  // create_program("kernel.cl", "");
	 create_program("kernel2.cl", "");

	cl_mem dev_result = create_result_buffer(num_bodies);
cl_mem crnl_pos = clCreateBuffer(g_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_float3)*num_bodies, host_pos, &error);
    cl_mem crnl_speed = clCreateBuffer(g_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_float3)*num_bodies, host_speed, &error);

	int is_done = 0;
	int tel = 0;
	time_measure_start("einde");
        while (!is_done & tel < 10) {
		time_measure_start("total");
		is_done=render_point_cloud(host_pos, num_bodies);
		simulate_gravity(host_pos, host_speed, num_bodies, dev_result, crnl_pos, crnl_speed);
		time_measure_stop_and_print("total");
		tel++;
	}
	time_measure_stop_and_print("einde");
}
