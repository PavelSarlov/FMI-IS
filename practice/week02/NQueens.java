import java.util.Scanner;

public class NQueens {

	public static void main(String[] args) {
		Scanner input = new Scanner(System.in);
		System.out.print("Enter N:");
		int n = input.nextInt();
		nQueens(n);

	}
	
	private static void nQueens(int n) {
		
//		1. n is even but not of the form (n mod 6 = 2).
//		Place queens on the squares
//		(m, 2m) and (n/2 +m, 2m-1) for m = 1, 2, . . . , n/2

//		2. n is even but not of the form (n mod 6 = 0) and
//		Place queens on the squares
//		(m, 1+(2(m-1)+ n/2 - 1)mod n) and
//		(n+1-m, n-(2(m-1)+n/2 -1)mod n) for m = 1,2,...,n/2

//		3. n is odd.
//		Use (1) or (2), whichever is appropriate, on n - 1 and extend with a queen at (n,n).
		
		if (n % 2 == 0) {
			if (!(n % 6 == 2)) {
				methodOne(n);
			} else if (!(n % 6 == 0)) {
				methodTwo(n);
			} else {
				System.err.println("ERROR !!!");
			}
		} else {
			methodThree(n);
		}
		
	}

	private static void methodOne(int n) {
//		1. n is even but not of the form (n mod 6 = 2).
//		Place queens on the squares
//		(m, 2m) and (n/2 +m, 2m-1) for m = 1, 2, . . . , n/2
		for (int m = 1; m <= n/2; m++) {
			for (int i = 1; i <= n; i++) {
				if (i == 2*m) {
					printQueen(true);
				} else {
					printQueen(false);
				}
			}
			System.out.println();
		}
		for (int m = 1; m <= n/2; m++) {
			for (int i = 1; i <= n; i++) {
				if (i == (2*m - 1)) {
					printQueen(true);
				} else {
					printQueen(false);
				}
			}
			System.out.println();
		}
	}
	
	private static void printQueen(boolean isQueen) {
		if (isQueen) {
			System.out.print("Q ");
		} else {
			System.out.print("_ ");
		}
	}

	private static void methodTwo(int n) {
//		2. n is even but not of the form (n mod 6 = 0) and
//		Place queens on the squares
//		(m, 1+(2(m-1)+ n/2 - 1)mod n) and
//		(n+1-m, n-(2(m-1)+n/2 -1)mod n) for m = 1,2,...,n/2
		for (int m = 1; m <= n/2; m++) {
			for (int i = 1; i <= n; i++) {
				if (i == (1 + (2 * (m - 1) + n/2 - 1) % n)) {
					printQueen(true);
				} else {
					printQueen(false);
				}
			}
			System.out.println();
		}
		for (int m = n/2; m >= 1; m--) {
			for (int i = 1; i <= n; i++) {
				if (i == (n - (2 * (m - 1) + n/2 - 1) % n)) {
					printQueen(true);
				} else {
					printQueen(false);
				}
			}
			System.out.println();
		}
	}
	
	private static void methodThree(int n) {
		methodOne(n-1);
		for (int i = 1; i <= n; i++) {
			if (i == n) {
				printQueen(true);
			} else {
				printQueen(false);
			}
		}
	}

}