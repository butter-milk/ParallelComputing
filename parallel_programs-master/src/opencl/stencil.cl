__kernel void stencil(
   __global float* input,
   __global float* output,
   const unsigned int count)
{
   int i = get_global_id(0)+1;
   output[i] = 0.1*input[i-1] + 0.2*input[i]+ 0.3*input[i+1];
}

