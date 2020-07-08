#include "pch.h"
#include <iomanip>
#include "Map.h"
#include "MMap.h"
using namespace std;

void test_BPTree_auto(int tree_size = 5000, int how_many = 500, bool report = false);
bool test_BPTree_auto(int how_many, bool report = true);


template <typename T>
void menu()
{
	Random r;
	BPTree<T> b;
	BPTree<T> c;
	char in;
	T val = T();
	while (true)
	{
		cout << "----------------------------------" << endl;
		cout << "[I]nsert [F]ind [S]ize [R]andom [E]rase [C]lear [T]est copy e[X]it" << endl;
		cin >> in;
		switch (in)
		{
		case 'I':cout << "Enter value" << endl;
			cin >> val;
			b.insert(val); break;
		case 'F':cout << "Enter value" << endl;
			cin >> val;
			if (b.find(val) == NULL) cout << "Key not found" << endl;
			else cout << *b.find(val) << " Found" << endl; break;
		case 'S': cout << b.size() << " Keys found" << endl; break;
		case 'R':val = r.GetNext(1, 100);
			cout << "Inserting " << val << endl;
			b.insert(val); break;
		case 'E': cout << "Enter value" << endl;
			cin >> val;
			b.remove(val); break;
		case 'C': b.clear_tree(); break;
		case 'T': c = b; c.print_debug(); cout << endl; c.is_valid_leaves(); break;
		case 'X': return;
		}
		b.print_debug();
		if (b.is_valid()) cout << "VALID" << endl;
		else cout << "NOT VALID" << endl;
	}
}

void shuffle(int a[], const int how_many)
{
	Random r;
	for (int i = 0; i < how_many; i++) {
		int from = r.GetNext(0, how_many - 1);
		int to = r.GetNext(0, how_many - 1);
		int temp = a[to];
		a[to] = a[from];
		a[from] = temp;
	}
}

void test_BPTree_auto(int tree_size, int how_many, bool report) {
	bool verified = true;
	for (int i = 0; i < how_many; i++) {
		if (report) {
			cout << "*********************************************************" << endl;
			cout << " T E S T:    " << i << endl;
			cout << "*********************************************************" << endl;
		}
		if (!test_BPTree_auto(tree_size, report)) {
			cout << "T E S T :   [" << i << "]    F A I L E D ! ! !" << endl;
			verified = false;
			return;
		}

	}
	cout << "**************************************************************************" << endl;
	cout << "**************************************************************************" << endl;
	cout << "             E N D     T E S T: " << how_many << " tests of " << tree_size << " items: ";
	cout << (verified ? "VERIFIED" : "VERIFICATION FAILED") << endl;
	cout << "**************************************************************************" << endl;
	cout << "**************************************************************************" << endl;
}

bool test_BPTree_auto(int how_many, bool report) {
	Random r;
	const int MAX = 10000;
	assert(how_many < MAX);
	Map<int,int> m;
	int a[MAX];
	int original[MAX];
	int deleted_list[MAX];

	int original_size;
	int size;
	int deleted_size = 0;

	//fill a[ ]
	for (int i = 0; i < how_many; i++) {
		a[i] = i;
	}
	shuffle(a, how_many);

	//copy  a[ ] -> original[ ]:
	copy_array(a, how_many, original, how_many);

	size = how_many;
	original_size = how_many;
	for (int i = 0; i < size; i++) {
		m.insert(a[i],0);

	}

	if (report) {
		cout << "========================================================" << endl;
		cout << "  " << endl;
		cout << "========================================================" << endl;
		cout << m << endl << endl;
	}

	for (int i = 0; i < how_many; i++) {
		int rand = r.GetNext(0, how_many - i - 1);
		if (report) {
			cout << "========================================================" << endl;
			cout << m << endl;
			cout << ". . . . . . . . . . . . . . . . . . . . . . . . . . . . " << endl;
			cout << "deleted: "; print_array(deleted_list, deleted_size);
			cout << "   from: "; print_array(original, original_size);
			cout << endl;
			cout << "  REMOVING [" << a[rand] << "]" << endl;
			cout << "========================================================" << endl;
		}
		m.erase(a[rand]);


		delete_item(a, rand, size, deleted_list[deleted_size++]);
		if (!m.is_valid()) {
			cout << setw(6) << i << " I N V A L I D   T R E E" << endl;
			cout << "Original Array: "; print_array(original, original_size);
			cout << "Deleted Items : "; print_array(deleted_list, deleted_size);
			cout << endl << endl << m << endl << endl;
			return false;
		}

	}

	if (report) cout << " V A L I D    T R E E" << endl;

	return true;
}

