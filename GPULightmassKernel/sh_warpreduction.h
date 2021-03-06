#pragma once

__inline__ __device__
float warpReduceSum(float val) {
	for (int offset = warpSize / 2; offset > 0; offset /= 2)
		val += __shfl_down_sync(__activemask(), val, offset);
	return val;
}

__inline__ __device__
float blockReduceSumToThread0(float val) {

	static __shared__ float shared[32]; // Shared mem for 32 partial sums
	int lane = (threadIdx.y * blockDim.x + threadIdx.x) % warpSize;
	int wid = (threadIdx.y * blockDim.x + threadIdx.x) / warpSize;

	val = warpReduceSum(val);     // Each warp performs partial reduction

	if (lane == 0) shared[wid] = val; // Write reduced value to shared memory

	__syncthreads();              // Wait for all partial reductions

								  //read from shared memory only if that warp existed
	val = ((threadIdx.y * blockDim.x + threadIdx.x) < blockDim.x * blockDim.y / warpSize) ? shared[lane] : 0;

	if (wid == 0) val = warpReduceSum(val); //Final reduce within first warp

	return val;
}

__device__ SHVector2 blockReduceSumToThread0(SHVector2 val)
{
	val.v[0] = blockReduceSumToThread0(val.v[0]);
	val.v[1] = blockReduceSumToThread0(val.v[1]);
	val.v[2] = blockReduceSumToThread0(val.v[2]);
	val.v[3] = blockReduceSumToThread0(val.v[3]);
	return val;
}

__device__ SHVectorRGB blockReduceSumToThread0(SHVectorRGB val)
{
	val.r = blockReduceSumToThread0(val.r);
	val.g = blockReduceSumToThread0(val.g);
	val.b = blockReduceSumToThread0(val.b);
	return val;
}

__device__ float3 blockReduceSumToThread0(float3 val)
{
	val.x = blockReduceSumToThread0(val.x);
	val.y = blockReduceSumToThread0(val.y);
	val.z = blockReduceSumToThread0(val.z);
	return val;
}

__device__ GPULightmass::GatheredLightSample blockReduceSumToThread0(GPULightmass::GatheredLightSample val)
{
	val.IncidentLighting = blockReduceSumToThread0(val.IncidentLighting);
	val.SHCorrection = blockReduceSumToThread0(val.SHCorrection);
	val.SHVector = blockReduceSumToThread0(val.SHVector);
	val.SkyOcclusion = blockReduceSumToThread0(val.SkyOcclusion);
	val.NumBackfaceHits = blockReduceSumToThread0(val.NumBackfaceHits);
	return val;
}
