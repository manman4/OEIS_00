package main 

import "fmt"

func tribo(n int) int {
    x, y, z := 0, 0, 1
    for i := 0; i < n; i++ {
        x, y, z = y, z, x+y+z
    }
    return x
}

func main() {
    fmt.Println(tribo(30))
}