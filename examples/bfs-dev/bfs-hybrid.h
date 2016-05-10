#include <vector>
#include <stdio.h>


//#//////////////////////////////////////////////
/// Breath-First-Search class.
/// The class is meant for executing many BFSs over a fixed graph. This means that the class can keep the hash tables and queues initialized between different calls of the DoBfs() function.
template<class PGraph>
class TBreathFS_Hybrid {
public:
  PGraph Graph;
  TInt StartNId;
  TIntV NIdDistV;
public:
  TBreathFS_Hybrid(const PGraph& GraphPt, const bool& InitBigV=true) :
    Graph(GraphPt), NIdDistV(Graph->GetMxNId()+1), InitBigV(InitBigV) {
    for (int i = 0; i < NIdDistV.Len(); i++) {
      NIdDistV.SetVal(i, -1);
    }
  }
  /// Performs BFS from node id StartNode for at maps MxDist steps by only following in-links (parameter FollowIn = true) and/or out-links (parameter FollowOut = true).
  int DoBfs_Hybrid(const int& StartNode, const bool& FollowOut, const bool& FollowIn, const int& TargetNId=-1, const int& MxDist=TInt::Mx);
private:
  bool InitBigV;
  /* Private functions */
  bool TopDownStep(TIntV *Frontier, TIntV *NextFrontier, int& MaxDist, const int& TargetNId);
  bool BottomUpStep(TIntV *Frontier, TIntV *NextFrontier, int& MaxDist, const int& TargetNId);
  int frontierEdges(TIntV *Frontier);
  int unvisitedEdges();
};

template<class PGraph>
int TBreathFS_Hybrid<PGraph>::DoBfs_Hybrid(const int& StartNode, const bool& FollowOut, const bool& FollowIn, const int& TargetNId, const int& MxDist) {
  StartNId = StartNode;
  IAssert(Graph->IsNode(StartNId));
  const typename PGraph::TObj::TNodeI StartNodeI = Graph->GetNI(StartNode);
  IAssertR(StartNodeI.GetOutDeg() > 0, TStr::Fmt("No neighbors from start node %d.", StartNode));

  TIntV *Frontier = new TIntV(InitBigV ? Graph->GetNodes() : 1024, 0);
  TIntV *NextFrontier = new TIntV(InitBigV ? Graph->GetNodes() : 1024, 0);

  NIdDistV.SetVal(StartNId, 0);
  Frontier->Add(StartNId);
  int MaxDist = -1;
  int TotalNodes = Graph->GetNodes();
  int Visited = 0;
  int Stage = 0; // 0, 2: top down, 1: bottom up
  while (! Frontier->Empty()) {
    MaxDist += 1;
    Visited += Frontier->Len();
//    int Unvisited = TotalNodes - Visited;

    NextFrontier->Clr(false);
    if (MaxDist == MxDist) { break; } // max distance limit reached

    if (Stage == 0 && (float)unvisitedEdges() / frontierEdges(Frontier) < 10) {
      printf("Changed to bottom up at step %d\n", MaxDist);
      Stage = 1;
    } else if (Stage == 1 && (float)TotalNodes / Frontier->Len() > 20) {
      printf("Changed back to top down at step %d\n", MaxDist);
      Stage = 2;
    }
    // Top down or bottom up depending on stage
    bool targetFound = false;
    if (Stage == 0 || Stage == 2) {
      targetFound = TopDownStep(Frontier, NextFrontier, MaxDist, TargetNId);
    } else {
      targetFound = BottomUpStep(Frontier, NextFrontier, MaxDist, TargetNId);
    }
    if (targetFound) break;

    // swap Frontier and NextFrontier
    TIntV *temp = Frontier;
    Frontier = NextFrontier;
    NextFrontier = temp;
  }
  
  delete Frontier;
  delete NextFrontier;

  return MaxDist;
}

template<class PGraph>
bool TBreathFS_Hybrid<PGraph>::TopDownStep(TIntV *Frontier, TIntV *NextFrontier, int& MaxDist, const int& TargetNId) {
  for (TIntV::TIter it = Frontier->BegI(); it != Frontier->EndI(); ++it) { // loop over frontier
    const int NId = *it;
    const int Dist = NIdDistV[NId];
    IAssert(Dist == MaxDist); // Must equal to MaxDist
    const typename PGraph::TObj::TNodeI NodeI = Graph->GetNI(NId);
    for (int v = 0; v < NodeI.GetOutDeg(); v++) {
      const int NeighborNId = NodeI.GetOutNId(v);
      if (NIdDistV[NeighborNId] == -1) {
        NIdDistV.SetVal(NeighborNId, Dist+1);
        if (NeighborNId == TargetNId) return true;
        NextFrontier->Add(NeighborNId);
        /* Remove */
        //childCounts[MaxDist].claimed += 1;
      }
    }
  }
  return false;
}

template<class PGraph>
bool TBreathFS_Hybrid<PGraph>::BottomUpStep(TIntV *Frontier, TIntV *NextFrontier, int& MaxDist, const int& TargetNId) {
  for (TNGraph::TNodeI NodeI = Graph->BegNI(); NodeI < Graph->EndNI(); NodeI++) {
    const int NId = NodeI.GetId();
    if (NIdDistV[NId] == -1) {
      for (int v = 0; v < NodeI.GetInDeg(); v++) {
        const int ParentNId = NodeI.GetInNId(v);
        if (NIdDistV[ParentNId] == MaxDist) {
          NIdDistV[NId] = MaxDist + 1;
          if (NId == TargetNId) return true;
          NextFrontier->Add(NId);
          /* Remove */
          //childCounts[MaxDist].claimed += 1;
          break;
        }
      }
    }
  }
  return false;
}

template<class PGraph>
int TBreathFS_Hybrid<PGraph>::frontierEdges(TIntV *Frontier) {
  int edges = 0;
  for (TIntV::TIter it = Frontier->BegI(); it != Frontier->EndI(); ++it) {
    const int NId = *it;
    const typename PGraph::TObj::TNodeI NodeI = Graph->GetNI(NId);
    edges += NodeI.GetOutDeg();
  }
  return edges;
}

template<class PGraph>
int TBreathFS_Hybrid<PGraph>::unvisitedEdges() {
  int edges = 0;
  for (TNGraph::TNodeI NodeI = Graph->BegNI(); NodeI < Graph->EndNI(); NodeI++) {
    const int NId = NodeI.GetId();
    if (NIdDistV[NId] == -1) {
      edges += NodeI.GetInDeg();
    }
  }
  return edges;
}

