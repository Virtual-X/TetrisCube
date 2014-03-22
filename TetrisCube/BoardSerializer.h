#ifndef __BOARDSERIALIZER_H__
#define __BOARDSERIALIZER_H__

#include "Board.h"

#include <vector>

class BoardSerializer
{
public:
  string Serialize(const Board& board) {
    SizeX = int.Parse(size.Substring(0, 1));
    SizeY = int.Parse(size.Substring(1, 1));
    SizeZ = int.Parse(size.Substring(2, 1));
    // read pieces
  }

  Board Unserialize(const string& str) {
    SizeX = int.Parse(size.Substring(0, 1));
    SizeY = int.Parse(size.Substring(1, 1));
    SizeZ = int.Parse(size.Substring(2, 1));
    // read pieces

    return Board(LoadSize(str), LoadPieces(str));
  }

private:

  vector<int> size;
  vector<Piece> pieces;

  private bool[,] equalPiecesMatrix;
  private bool allPiecesDifferent;

  public bool AreAllPiecesDifferent() {
    if (equalPiecesMatrix == null)
      CalculateEqualPiecesMatrix();
    return allPiecesDifferent;
  }

  public bool AreEqualPiece(int pieceIndex1, int pieceIndex2) {
    if (equalPiecesMatrix == null)
      CalculateEqualPiecesMatrix();
    return equalPiecesMatrix[pieceIndex1, pieceIndex2];
  }

  private void CalculateEqualPiecesMatrix() {
    allPiecesDifferent = true;
    equalPiecesMatrix = new bool[Pieces.Length, Pieces.Length];
    for (int i = 0; i < Pieces.Length; i++) {
      var p1 = Pieces[i];
      for (int j = 0; j < Pieces.Length; j++) {
        var p2 = Pieces[j];
        var equal = i == j;
        if (!equal && AreEqualPieces(p1, p2)) {
          allPiecesDifferent = false;
          equal = true;
        }
        equalPiecesMatrix[i, j] = equal;
      }
    }
  }

  private bool AreEqualPieces(Piece p1, Piece p2) {
    if (p1.Color != p2.Color)
      return false;
    var c1 = p1.GetCoords(0);
    if (c1.Length != p2.GetCoords(0).Length)
      return false;
    for (int i = 0; i < p2.OrientationsCount; i++) {
      if (Geometry.AreEqual(c1, p2.GetCoords(i)))
        return true;
    }
    return false;
  }
};


#endif // __BOARD_SERIALIZER_H__