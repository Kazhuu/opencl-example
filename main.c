#include <CL/cl.h>
#define CL_TARGET_OPENCL_VERSION 120
#include <CL/opencl.h>
#include <stdio.h>
#include <stdbool.h>

static const char* program_source =
"__kernel void                                                                  "
"vector_add (__global const uint *a, __global const uint *b, __global uint *c)  "
"{                                                                              "
"  int gid = get_global_id(0);                                                  "
"  uint prod = a[gid] + b[gid];                                                 "
"  c[gid] = prod;                                                               "
"}                                                                              ";

int main(int argc, char **argv)
{
    int *src_a, *src_b;
    int *destination;
    const int elements = 10;

    size_t data_size = sizeof(int)*elements;

    src_a = (int*)malloc(data_size);
    src_b = (int*)malloc(data_size);
    destination = (int*)malloc(data_size);

    for (int i = 0; i < elements; ++i)
    {
        src_a[i] = i;
        src_b[i] = i;
    }

    // STEP 1: Discover and initialize the platform.
    cl_int status;
    cl_uint platform_count = 0;
    cl_platform_id *platforms = NULL;

    // Use clGetPlatformIDs() to retrieve the number of platforms.
    status = clGetPlatformIDs(0, NULL, &platform_count);
    if (status != CL_SUCCESS) {
        printf("get number of platforms error\n");
        goto allocate_arrays;
    }
    printf("found %d platforms\n", platform_count);

    // Allocate space for found platforms.
    platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * platform_count);

    // Fill in platforms with clGetPlatformIDs().
    status = clGetPlatformIDs(platform_count, platforms, NULL);
    if (status != CL_SUCCESS) {
        printf("fill in platforms error\n");
        goto allocate_platforms;
    }

    // STEP 2: Discover and initialize the devices.
    cl_uint device_count = 0;
    cl_device_id* devices = NULL;

    // Use clGetDeviceIDs() to retrieve number of devices on the first platform.
    status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, NULL, &device_count);
    if (status != CL_SUCCESS) {
        printf("get number of devices error\n");
        goto allocate_platforms;
    }
    printf("found %d devices\n", device_count);

    // Allocate space for devices.
    devices = (cl_device_id*)malloc(sizeof(cl_device_id) * device_count);

    // Fill in devices with clGetDeviceIDs().
    status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, device_count, devices, NULL);
    if (status != CL_SUCCESS) {
        printf("fill in devices error\n");
        goto allocate_devices;
    }

    // STEP 3: Create a context.
    cl_context context = NULL;

    // Create a context using clCreateContext() and associate it with the
    // devices.
    context = clCreateContext(NULL, device_count, devices, NULL, NULL, &status);
    if (status != CL_SUCCESS) {
        printf("create context error\n");
        goto allocate_devices;
    }

    // STEP 4: Create a command queue.
    cl_command_queue command_queue;

    // Create a command queue using clCreateCommandQueue() and associate it with
    // the device you want to execute on.
    command_queue = clCreateCommandQueue(context, devices[0], 0, &status);
    if (status != CL_SUCCESS) {
        printf("create command queue error\n");
        goto allocate_context;
    }

    // STEP 5: Create device buffers.
    cl_mem buffer_a;
    cl_mem buffer_b;
    cl_mem buffer_c;

    // Use clCreateBuffer() to create a buffer objects that will contain the data
    // from the host to device and back to host. Repeat following for all three
    // arrays.
    buffer_a = clCreateBuffer(context, CL_MEM_READ_ONLY, data_size, NULL, &status);
    if (status != CL_SUCCESS) {
        printf("create buffer_a error\n");
        goto allocate_command_queue;
    }
    buffer_b = clCreateBuffer(context, CL_MEM_READ_ONLY, data_size, NULL, &status);
    if (status != CL_SUCCESS) {
        printf("create buffer_b error\n");
        goto allocate_buffer_a;
    }
    buffer_c = clCreateBuffer(context, CL_MEM_WRITE_ONLY, data_size, NULL, &status);
    if (status != CL_SUCCESS) {
        printf("create buffer_c error\n");
        goto allocate_buffer_b;
    }

    // STEP 6: Write host data to device buffers.
    // use clEnqueueWriteBuffer() to write input array to the device buffer.
    // Repeat this for two device in put arrays.
    status = clEnqueueWriteBuffer(command_queue, buffer_a, CL_FALSE, 0, data_size, src_a, 0, NULL, NULL);
    if (status != CL_SUCCESS) {
        printf("write buffer_a error\n");
        goto allocate_buffer_c;
    }
    status = clEnqueueWriteBuffer(command_queue, buffer_b, CL_FALSE, 0, data_size, src_b, 0, NULL, NULL);
    if (status != CL_SUCCESS) {
        printf("write buffer_b error\n");
        goto allocate_buffer_c;
    }

    // STEP 7: Create and compile the program.
    // Create a program using clCreateProgramWithSources().
    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&program_source, NULL, &status);
    if (status != CL_SUCCESS) {
        printf("create program error\n");
        goto allocate_buffer_c;
    }

    // Compile the program for the devices with clBuildProgram().
    status = clBuildProgram(program, device_count, devices, NULL, NULL, NULL);
    if (status != CL_SUCCESS) {
        printf("compile program error\n");
        goto allocate_program;
    }

    // STEP 8: Create the kernel.
    cl_kernel kernel = NULL;

    // Use clCreateKernel() to create a kernel from the vector addition function
    // named "vector_add".
    kernel = clCreateKernel(program, "vector_add", &status);
    if (status != CL_SUCCESS) {
        printf("create kernel error\n");
        goto allocate_program;
    }

    // STEP 9: Set the kernel arguments.
    // Associate the input and output buffers with the kernel using
    // clSetKernelArg().
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_a);
    if (status != CL_SUCCESS) {
        printf("set arg 0 error\n");
        goto allocate_kernel;
    }
    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_b);
    if (status != CL_SUCCESS) {
        printf("set arg 1 error\n");
        goto allocate_kernel;
    }
    status = clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffer_c);
    if (status != CL_SUCCESS) {
        printf("set arg 2 error\n");
        goto allocate_kernel;
    }

    // STEP 10: Configure the work-item structure.
    // Define an index space (global work size) of work items for execution. A
    // workgroup size (local work size) is not required, but can be used.
    size_t global_work_size[1];
    // There are number of elements of work-items.
    global_work_size[0] = elements;

    // STEP 11: Enqueue the kernel for execution.
    // Execute the kernel by using clEnqueuNDRangeKernel().
    status = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
    if (status != CL_SUCCESS) {
        printf("kernel execute error\n");
        goto allocate_kernel;
    }

    // STEP 12: Read the output buffer back to the host.
    // Use clEnqueueReadBuffer() to read the OpenCL output buffer (buffer_c) to the host
    // output array (destination).
    clEnqueueReadBuffer(command_queue, buffer_c, CL_TRUE, 0, data_size, destination, 0, NULL, NULL);

    // Verify the output.
    bool result = true;
    for (int i = 0; i < elements; ++i) {
        if (destination[i] != i + i) {
            result = false;
        }
    }
    if (result) {
        printf("ouput is correct\n");
    } else {
        printf("ouput is incorrect\n");
    }

allocate_kernel:
    clReleaseKernel(kernel);
allocate_program:
    clReleaseProgram(program);
allocate_buffer_c:
    clReleaseMemObject(buffer_c);
allocate_buffer_b:
    clReleaseMemObject(buffer_b);
allocate_buffer_a:
    clReleaseMemObject(buffer_a);
allocate_command_queue:
    clReleaseCommandQueue(command_queue);
allocate_context:
    clReleaseContext(context);
allocate_devices:
    free(devices);
allocate_platforms:
    free(platforms);
allocate_arrays:
    free(src_a);
    free(src_b);
    free(destination);

    return 0;
}
