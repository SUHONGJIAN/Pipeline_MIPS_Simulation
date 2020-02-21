#include<iostream>
#include<bitset>
#include<string>
using namespace std;



int main()
{
	bitset<5> shj = bitset<5> ("10111");
	bitset<8> hhh = 224;
	int x = 9;
	bool y = true;
	string stringTest = "32956";
	for(int i = 0; i < hhh.size(); i++) {
		cout<<hhh[i]<<endl;
	}
	for(int i = 0; i < stringTest.size(); i++) {
		cout<<stringTest[i]<<endl;
	}
	int m = 0x23;
	if(x) cout<<x<<endl;
	if(y) cout<<y<<endl;
	cout<<m<<endl;
}
