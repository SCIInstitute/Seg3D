//
//  HeapNode.h
//  
//
//  Created by Salma Bengali on 4/15/14.
//
//

#ifndef ____HeapNode__
#define ____HeapNode__

#include "fibheap.h"
/***************************************************************************
 * class HeapNode
 ***************************************************************************/
namespace FGC {
    class HeapNode : public FibHeapNode {
        float   N;
        long IndexV;
        
    public:
        HeapNode() : FibHeapNode() { N = 0; }
        virtual void operator =(FibHeapNode& RHS);
        virtual int  operator ==(FibHeapNode& RHS);
        virtual int  operator <(FibHeapNode& RHS);
        virtual void operator =(double NewKeyVal );
        virtual void Print();
        double GetKeyValue() { return N; }
        void SetKeyValue(double n) { N = n; }
        long int GetIndexValue() { return IndexV; }
        void SetIndexValue( long int v) { IndexV = v; }
    };
}

#endif /* defined(____HeapNode__) */
