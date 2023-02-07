#include <iostream>
#include <fstream>
#include <omp.h>
#include <vector>
#include <Math.h>

using namespace std;

typedef double(*TestFunctTempl)(vector<double>&, vector<double>&, vector<double>&, int&);

double FillArrayConsistent(vector<double>& a, int& size)
{
	double t_start = omp_get_wtime();
	for (int i = 0; i < size; i++)
	{
		a[i] = (sin(i - 50) + cos(i / 2));
	}
	double t_end = omp_get_wtime();
	return t_end - t_start;
}
double FillArrayParallel(vector<double>& a, int& size)
{
	double t_start = omp_get_wtime();
#pragma omp parallel for

	for (int i = 0; i < size; i++)
	{
		a[i] = (sin(i - 50) + cos(i / 2));
	}

	double t_end = omp_get_wtime();
	return t_end - t_start;
}

double SumArrayConsistent(vector<double> a, vector<double> b, vector<double>& c, int size)
{
	double t_start = omp_get_wtime();
	for (int i = 0; i < size; i++)
	{
		c[i] = a[i] + b[i];
	}
	double t_end = omp_get_wtime();
	return t_end - t_start;
}
double SumArrayParallel(vector<double> a, vector<double> b, vector<double>& c, int size)
{
	double t_start = omp_get_wtime();
#pragma omp parallel for 

	for (int i = 0; i < size; i++)
	{
		c[i] = a[i] + b[i];
	}

	double t_end = omp_get_wtime();
	return t_end - t_start;
}
double SumArraySections(vector<double> a, vector<double> b, vector<double>& c, int size)
{
	double t_start = omp_get_wtime();
	c.clear();
	int n_t, i = 0;
#pragma omp parallel 
	{
		n_t = omp_get_max_threads();
	}
	int st = 0, st1 = size / n_t, st2 = size * 2 / n_t, st3 = size * 3 / n_t, se = size;
#pragma omp parallel sections private(i)
	{
#pragma omp section
		{
			for (i = st; i < st1; i++)
			{
				c[i] = a[i] + b[i];
			}
		}
#pragma omp section
		{
			for (i = st1; i < st2; i++)
			{
				c[i] = a[i] + b[i];
			}
		}
#pragma omp section
		{
			if (n_t > 2)
			{
				for (i = st2; i < st3; i++)
				{
					c[i] = a[i] + b[i];
				}
			}
		}
#pragma omp section
		{
			if (n_t > 3)
			{
				for (i = st3; i < se; i++)
				{
					c[i] = a[i] + b[i];
				}
			}
		}
	}
	double t_end = omp_get_wtime();
	return t_end - t_start;
}

double SumResultConsistent(vector<double>& c, double sum, int size)
{
	double t_start = omp_get_wtime();
	for (int i = 0; i < size; i++)
	{
		sum += c[i];
	}
	double t_end = omp_get_wtime();
	return t_end - t_start;
}
double SumResultReduction(vector<double>& c, double sum, int size)
{
	double t_start = omp_get_wtime();
#pragma omp parallel for reduction(+:sum) 

	for (int i = 0; i < size; i++)
	{
		sum += c[i];
	}

	double t_end = omp_get_wtime();
	return t_end - t_start;
}
double sumResultCritical(vector<double>& c, double sum, int size)
{
	double t_start = omp_get_wtime();
	double temp = 0;
#pragma omp parallel for
	for (int i = 0; i < size; i++)
#pragma omp critical
		temp += c[i];
	sum = temp;
	double t_end = omp_get_wtime();
	return t_end - t_start;
}

double TestFillConsistent(vector<double>& a, vector<double>& empty1, vector<double>& empty2, int& size)
{
	return FillArrayConsistent(a, size);
}
double TestFillParallel(vector<double>& a, vector<double>& empty1, vector<double>& empty2, int& size)
{
	return FillArrayParallel(a, size);
}

double TestSumConsistent(vector<double>& a, vector<double>& b, vector<double>& c, int& size)
{
	return SumArrayConsistent(a, b, c, size);
}
double TestSumParallel(vector<double>& a, vector<double>& b, vector<double>& c, int& size)
{
	return SumArrayParallel(a, b, c, size);
}
double TestSumSections(vector<double>& a, vector<double>& b, vector<double>& c, int& size)
{
	return SumArraySections(a, b, c, size);
}

double TestSumResultConsistent(vector<double>& empty1, vector<double>& empty2, vector<double>& c, int& size)
{
	double sum = 0;
	return SumResultConsistent(c, sum, size);
}
double TestSumResultReduction(vector<double>& empty1, vector<double>& empty2, vector<double>& c, int& size)
{
	double sum = 0;
	return SumResultReduction(c, sum, size);
}
double TestSumResultCritical(vector<double>& empty1, vector<double>& empty2, vector<double>& c, int& size)
{
	double sum = 0;
	return sumResultCritical(c, sum, size);
}


double AvgTrustedInterval(double& avg, vector<double>& times, int& cnt)
{
	double sd = 0, newAVg = 0;
	int newCnt = 0;
	for (int i = 0; i < cnt; i++)
	{
		sd += (times[i] - avg) * (times[i] - avg);
	}
	sd /= (cnt - 1.0);
	sd = sqrt(sd);
	for (int i = 0; i < cnt; i++)
	{
		if (avg - sd <= times[i] && times[i] <= avg + sd)
		{
			newAVg += times[i];
			newCnt++;
		}
	}
	if (newCnt == 0) newCnt = 1;
	return newAVg / newCnt;
}
double TestIter(void* Funct, vector<double>& a, vector<double>& b, vector<double>& c, int size, int iterations)
{
	double curtime = 0, avgTime = 0, avgTimeT = 0, correctAVG = 0;;
	vector<double> Times(iterations);
	cout << endl;
	for (int i = 0; i < iterations; i++)
	{
		// Запуск функции и получение времени в миллисекундах
		curtime = ((*(TestFunctTempl)Funct)(a, b, c, size)) * 1000;
		// запись времени в массив для определения среднеарифметического значения в доверительном интервале
		Times[i] = curtime;
		avgTime += curtime;
		cout << "+";
	}
	cout << endl;
	// Вычисление среднеарифметического по всем итерациям и вывод значения на экран
	avgTime /= iterations;
	cout << "AvgTime:" << avgTime << endl;
	// Определения среднеарифметического значения в доверительном интервале по всем итерациям и вывод значения на экран
	avgTimeT = AvgTrustedInterval(avgTime, Times, iterations);
	cout << "AvgTimeTrusted:" << avgTimeT << endl;
	return avgTimeT;
}

void test_functions(void** Functions, vector<string> fNames)
{
	vector<double> a, b, c;
	int iters = 100;
	int nd = 0;
	double times[4][8][3];
	for (int size = 100000; size <= 250000; size += 50000)
	{
		a.resize(size);
		b.resize(size);
		c.resize(size);
		FillArrayConsistent(b, size);
		for (int threads = 1; threads <= 4; threads++)
		{
			omp_set_num_threads(threads);
			//перебор алгоритмов по условиям
			for (int alg = 0; alg <= 7; alg++)
			{
				if (threads == 1)
				{
					if (alg == 0 || alg == 2 || alg == 5) {
						times[nd][alg][0] = TestIter(Functions[alg], a, b, c, size, iters);
						// iters - кол-во запусков алгоритма
						times[nd][alg][1] = times[nd][alg][0];
						times[nd][alg][2] = times[nd][alg][0];
					}
				}
				else
				{
					if (alg != 0 && alg != 2 && alg != 5)
					{
						times[nd][alg][threads - 2] = TestIter(Functions[alg], a, b, c, size, iters);
					}
				}
			}
		}
		nd++;
	}

	ofstream fout("output.txt");
	for (int threads = 1; threads <= 4; threads++)
	{
		cout << "Thread " << threads << " --------------" << endl;

		for (int ND = 0; ND < 4; ND++)
		{
			switch (ND)
			{
			case 0:
				cout << "\n----------100000 elements of array----------" << endl;
				break;
			case 1:
				cout << "\n----------150000 elements of array----------" << endl;
				break;
			case 2:
				cout << "\n----------200000 elements of array----------" << endl;
				break;
			case 3:
				cout << "\n----------250000 elements of array----------" << endl;
				break;
			default:
				break;
			}


			for (int alg = 0; alg < 8; alg++)
			{
				if (threads == 1)
				{
					if (alg == 0 || alg == 2 || alg == 5) {
						cout << fNames[alg] << "\t" << times[ND][alg][0] << " ms." << endl;
						fout << times[ND][alg][0] << endl;
					}
				}
				else
				{
					if (alg != 0 && alg != 2 && alg != 5)
					{
						cout << fNames[alg] << "\t" << times[ND][alg][threads - 2] << " ms." << endl;
						fout << times[ND][alg][threads - 2] << endl;
					}
				}
			}
		}
	}
	fout.close();


}

int main()
{
	void** Functions = new void* [8] { FillArrayConsistent, FillArrayParallel, TestSumConsistent, TestSumParallel, TestSumSections, TestSumResultConsistent, TestSumResultReduction, TestSumResultCritical };
	vector<string> function_names = { "Consistent filling","Parallel filling using loop FOR",
		"Consistent sum two vectors",
		"Parallel sum two vectors using loop FOR", "Parallel sum two vectors using Sections",
		"Consistent sum the last vector", "Parallel sum last vector using Reduction",
		"Parallel sum the last vectors using Critical Sections" };
	test_functions(Functions, function_names);
	return 0;
}