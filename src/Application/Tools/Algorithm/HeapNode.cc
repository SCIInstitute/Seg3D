//
//  HeapNode.cpp
//  
//
//  Created by Salma Bengali on 4/15/14.
//
//

#include "HeapNode.h"

namespace FGC{
void HeapNode::Print() {
    FibHeapNode::Print();
    //    mexPrintf( "%f (%d)" , N , IndexV );
}

void HeapNode::operator =(double NewKeyVal) {
    HeapNode Tmp;
    Tmp.N = N = NewKeyVal;
    FHN_Assign(Tmp);
}

void HeapNode::operator =(FibHeapNode& RHS) {
    FHN_Assign(RHS);
    N = ((HeapNode&) RHS).N;
}

int  HeapNode::operator ==(FibHeapNode& RHS) {
    if (FHN_Cmp(RHS)) return 0;
    return N == ((HeapNode&) RHS).N ? 1 : 0;
}

int  HeapNode::operator <(FibHeapNode& RHS) {
    int X;
    if ((X=FHN_Cmp(RHS)) != 0)
        return X < 0 ? 1 : 0;
    return N < ((HeapNode&) RHS).N ? 1 : 0;
}
} // end namespace