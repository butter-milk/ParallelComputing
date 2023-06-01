__kernel void stencil(
   __global float* input,
   __global float* output,
   const unsigned int count)
{
   int i = get_global_id(0);
   output[i] = input[i] * input[i];
}

