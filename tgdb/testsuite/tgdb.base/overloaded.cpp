class A {
	public:
		void func ( int a );
		void func ( float a );
};

void A::func ( int a ) {}
void A::func ( float a ) {}

int main ( int argc, char **argv ) {

	return 0;
}
