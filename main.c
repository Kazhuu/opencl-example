#define CL_TARGET_OPENCL_VERSION 120
#include <CL/opencl.h>
#include <stdio.h>

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

    cl_int status;

    // Discover and initialize the platform.
    cl_uint platform_count = 0;
    cl_platform_id *platform = NULL;

    status = clGetPlatformIDs(0, NULL, &platform_count);
    printf("%d", platform_count);

    return 0;
}
