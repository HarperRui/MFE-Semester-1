#include <iostream>
#include <vector>
using namespace std;

//print the vector
void print(vector<double> &v){
    for (auto i:v){
        cout<< i<<" ";
    }
    cout << endl;
}
//swap the element in the vector
void swap(double &a, double &b){
    if(a > b){
        auto temp = a;
        a = b;
        b = temp;
    }
}
int pivot_idx(vector<double> &v, int left, int right){
    //the first element of the vector
    double pivot = v[left];
    int count = 0;
    //count the number of element < pivot
    for (int i = left + 1; i <= right; i++){
        if(v[i] <= pivot)
            count++;
    }

    int idx = left + count;
    swap(v[idx], v[left]);
    //sort the vector
    int i, j;
    i = left;
    j = right;
    while (i < idx && j > idx){
        while (v[i] <= pivot){
            i++;
        }
        while (v[j] > pivot){
            j--;
        }
        if (i < idx && j > idx) {
            swap(v[i++], v[j--]);
        }
    }

    return idx;
}

void quick_sort(vector<double> &v, int left, int right){
    if (left >= right){
        return;
    }
    int p = pivot_idx(v, left, right);

    //sort the left part
    quick_sort(v, left, p - 1);
    //sort the right part
    quick_sort(v, p + 1, right);

}
//Heap Sort
void heap_sort(vector<double>& v){
    make_heap(v.begin(), v.end());
    for (int i = v.size() - 1; i > -1; i--){
        swap(v[0], v[i]);
        make_heap(v.begin(), v.begin() + i);
    }
}


int main() {
    cout << "----Quicksort----" << endl;
    vector<double> v{2,7,5,9,4,6};
    cout<<"v: ";
    print(v);
    quick_sort(v, 0, v.size()-1);
    cout<<"v after quick sorting: ";
    print(v);
    cout << "----Heapsort----" << endl;
    vector<double> v2{2,7,5,9,4,6};
    cout<<"v2: ";
    print(v2);
    heap_sort(v2);
    cout<<"v after heap sorting: ";
    print(v2);

    return 0;
}
