#pragma once

struct PEINVENTORY_API FPe2DBinPackNode
{
	int32 LeftChild;
	int32 RightChild;
	
	FIntRect Rect;
	bool     bFilled;

	FPe2DBinPackNode() : LeftChild(-1), RightChild(-1), Rect(), bFilled(false) {}
	FPe2DBinPackNode(FIntRect InRect) : LeftChild(-1), RightChild(-1),  Rect(InRect), bFilled(false) {}

	bool HasChildren() const
	{
		return LeftChild >= 0 || RightChild >= 0;
	}

	bool CanFit(const FIntRect& TestRect) const
	{
		return TestRect.Width() <= Rect.Width() && TestRect.Height() <= Rect.Height();
	}

	bool IsPerfectFit(const FIntRect& TestRect) const
	{
		return TestRect.Width() == Rect.Width() && TestRect.Height() == Rect.Height();
	}
	
	static bool Insert(TArray<FPe2DBinPackNode>& Nodes, int32 IdxNode, const FIntRect& InputRect, FIntRect& OutRect, int32 Depth = 0)
	{
		FPe2DBinPackNode& Node = Nodes[IdxNode];
		
		if(Node.HasChildren())
		{
			const bool bFound = Insert(Nodes, Node.LeftChild, InputRect, OutRect, Depth++);
			if(!bFound) { return Insert(Nodes, Node.RightChild, InputRect, OutRect, ++Depth); }
			
			return true;
		}
		
		// Item already exists here or the input can't fit.
		if(Node.bFilled || !Node.CanFit(InputRect)) { return false; }

		// Is a perfect / snug fit.
		if(Node.IsPerfectFit(InputRect))
		{
			Node.bFilled = true;
			OutRect = Node.Rect;
			return true;
		}

		const int32 SlotWidth  = InputRect.Width();
		const int32 SlotHeight = InputRect.Height();
		const int32 WidthDiff  = Node.Rect.Width() -  SlotWidth;
		const int32 HeightDiff = Node.Rect.Height() - SlotHeight;

		FIntRect LeftRect(Node.Rect);
		FIntRect RightRect(Node.Rect);

		// Decide to split vertically or horizontally.
		if(WidthDiff > HeightDiff) // Vertical split.
		{
			LeftRect.Max.X   = LeftRect.Min.X + SlotWidth;
			RightRect.Min.X += SlotWidth;
		}
		else // Horizontal split.
		{
			LeftRect.Max.Y   = LeftRect.Min.Y + SlotHeight;
			RightRect.Min.Y += SlotHeight;
		}

		const int32 LeftChildId = Nodes.Num();

		Node.LeftChild  = LeftChildId;
		Node.RightChild = LeftChildId + 1;
		
		Nodes.Add(FPe2DBinPackNode(LeftRect));
		Nodes.Add(FPe2DBinPackNode(RightRect));

		return Insert(Nodes, LeftChildId, InputRect, OutRect, ++Depth);
	}
};