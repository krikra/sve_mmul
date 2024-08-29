#include <arm_sve.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <new>
#include <random>

constexpr std::size_t N = 1024;

void mmul_simd(const float * a, const float * b, float * c) {
	auto simd_width = svcntw();
	for(auto i = 0uz; i < N; ++i) {
	for(auto k = 0uz; k < N; ++k) {
		svfloat32_t aik = svdup_f32(a[i*N+k]);
	for(auto j = 0uz; j < N; j+=simd_width) {
		svfloat32_t bkj = svld1_f32(svptrue_b32(), b+k*N+j);
		svfloat32_t cij = svld1_f32(svptrue_b32(), c+i*N+j);
		cij = svmad_f32_z(svptrue_b32(), aik, bkj, cij);
		svst1_f32(svptrue_b32(), c+i*N+j, cij);
	}
	}
	}
}

void mmul_naive(const float * a, const float * b, float * c) {
	for(auto i = 0; i < N; ++i) {
	for(auto j = 0; j < N; ++j) {
	for(auto k = 0; k < N; ++k) {
		c[i*N+j] += a[i*N+k]*b[k*N+j];
	}
	}
	}
}

void kernel() {
	auto a = std::unique_ptr<float[]>(new(std::align_val_t{32}) float[N*N]);
	auto c = std::unique_ptr<float[]>(new(std::align_val_t{32}) float[N*N]);
	auto c2 = std::unique_ptr<float[]>(new(std::align_val_t{32}) float[N*N]);
	auto b = std::unique_ptr<float[]>(new(std::align_val_t{32}) float[N*N]);

	std::mt19937_64 rng(0xdeadbeefdeadbeef);
	std::uniform_real_distribution<float> runif(0, 1);
	for(auto i = 0; i < N*N; ++i) {
		a[i] = runif(rng);
		b[i] = runif(rng);
		c[i] = runif(rng);
		c2[i] = c[i];
	}

	const auto beg = std::chrono::high_resolution_clock::now();
	mmul_simd(a.get(), b.get(), c.get());
	const auto end = std::chrono::high_resolution_clock::now();

	const std::chrono::duration<double> t = end-beg;

	std::cout << t.count() << " [s]\n";
	std::cout << static_cast<double>(2*N*N*N) / t.count() * 1.e-9 << " [GFlops]\n";

	mmul_naive(a.get(), b.get(), c2.get());

	std::size_t num_oops = 0;
	for(auto i = 0; i < N*N; ++i) {
		if(1.e-8 < std::abs(c2[i] - c[i])) {
			++num_oops;
		}
	}

	std::cout << num_oops << " were erronous";
}


int main() {
	kernel();
	return 0;
}
