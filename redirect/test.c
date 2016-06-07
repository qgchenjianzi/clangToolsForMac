/**
 * File Name: ../input/test.c
 * Function Name: add
 * Returning Type: int
 */ 
int add(int a){
 //the return stmt
 return a;
}

/**
 * File Name: ../input/test.c
 * Function Name: B
 * Returning Type: void
 */ 
void B(){
  int a = 3;
  if(a!=2)
    // the 'if' part
    //the func--->add() begin called!
    add(a);
}


