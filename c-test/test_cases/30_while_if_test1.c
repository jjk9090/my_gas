int whileIf() {
  int a;
  a = 0;
  int b;
  b = 0;
  while (a < 100) {
    if (a == 5) {
      b = 25;
    }
    else {
      b = a * 2;
    }
    a = a + 1;
  }
  return (b);
}


int main(){
  return (whileIf());
}
