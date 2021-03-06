
#include "stdafx.h"
#include "util.h"
#include "word2vec.h"

word2vec::word2vec() {
	_is_init = false;
}

word2vec::~word2vec() {
	destroy();
}

void word2vec::init(long long V, long long D) {
	if (_is_init) destroy();
	float *ptr = new float[D*V * 4];
	_V = V;
	_D = D;
	_W1_transp = ptr;
	_W2_transp = ptr + (D*V);
	_grad_W1_L = ptr + (D*V * 2);
	_grad_W2_L = ptr + (D*V * 3);
	_e = 0.01f;

	for (long long i = 0; i < V*D; i++) {
		_W1_transp[i] = (float)((float)rand() / (float)(RAND_MAX) / 2) - 1;
	}
	for (long long i = 0; i < V*D; i++) {
		_W2_transp[i] = (float)((float)rand() / (float)(RAND_MAX) / 2) - 1;
	}

	_is_init = true;
}

void word2vec::load(const wchar_t *path) {
	if (_is_init) throw std::runtime_error(u8"word2vec: destroy before trying to load");
}

void word2vec::save(const wchar_t *path) {
	if (_is_init) throw std::runtime_error(u8"word2vec: cannot save as instance not initialized");
}

void word2vec::destroy() {
	if (_is_init) {
		delete[] _W1_transp;
		_is_init = false;
	}
}

void word2vec::set_learning_rate(float e) {
	_e = e;
}

float word2vec::step(const long long *in, const long long *out, long long ccExample) {
	if (!_is_init) {
		throw std::runtime_error(u8"word2vec: step() cannot be called before init");
	}
	memset(_grad_W1_L, 0, sizeof(float)*_D*_V);
	memset(_grad_W2_L, 0, sizeof(float)*_D*_V);

	float *hidden_layer = new float[_D];
	float *output_layer = new float[_V](); // set to zero
	float *softmax_layer = new float[_V];

	float loss = 0;

	for (long long _i = 0; _i < ccExample; _i++) {
		// forward propagation
		for (int i = 0; i < _D; i++) {
			hidden_layer[i] = _W1(in[_i], i);
		}
		memcpy(hidden_layer, _W1_transp + (_D*in[_i]), sizeof(float)*_D);
		for (long long v = 0; v < _V; v++) {
			for (long long d = 0; d < _D; d++) {
				output_layer[v] += _W2(v, d) * hidden_layer[d];
			}
		}
		softmax(output_layer, softmax_layer, _V);
		// backpropagation
		long long K = out[_i];
		loss += softmax_layer[K];
		float grad_a_L = -1 / softmax_layer[K];
		std::vector<float> grad_x2_L(_V);
		for (long long i = 0; i < _V; i++) {
			float grad_x2_a;
			if (i == K) {
				grad_x2_a = softmax_layer[i] * (1 - softmax_layer[i]);
			}
			else {
				grad_x2_a = -softmax_layer[i] * softmax_layer[K];
			}
			grad_x2_L[i] = grad_x2_a * grad_a_L;
		}
		std::vector<float> grad_x1_L(_D);
		for (long long i = 0; i < _D; i++) {
			float grad_x1_L_val = 0;
			for (long long j = 0; j < _V; j++) {
				float grad_x1_x2 = _W2(i, j);
				grad_x1_L_val += grad_x1_x2 * grad_x2_L[j];
			}
			grad_x1_L[i] = grad_x1_L_val;
		}
		for (long long i = 0; i < _V; i++) {
			for (long long j = 0; j < _D; j++) {
				// w2[i][j] only affects x2[i]
				float grad_w2_x2 = hidden_layer[j];
				_W2_grad(j, i) += grad_w2_x2 * grad_x2_L[i];
			}
		}
		for (long long i = 0; i < _D; i++) {
			for (long long j = 0; j < _V; j++) {
				// w1[i][j] only affects x1[i]
				float grad_w1_x1 = !!(j == K); // input layer (implicit)
				_W1_grad(j, i) += grad_w1_x1 * grad_x1_L[i];
			}
			//_W1_grad(K, i) += grad_x1_L[i];
		}
	}

	delete[] hidden_layer;
	delete[] output_layer;
	delete[] softmax_layer;

	add_batch(_W1_transp, _grad_W1_L, _D*_V, _e / ccExample);
	add_batch(_W2_transp, _grad_W2_L, _D*_V, _e / ccExample);

	return loss / ccExample;
}

void word2vec::add_batch(float *dst, float *src, long long count, float mul) {
	for (long long i = 0; i < count; i++) {
		*dst++ += mul * (*src++);
	}
}
