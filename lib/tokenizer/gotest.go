// How to compile: 'go build gotest.go'
// The executable would be gotest
package main

// hello
/* Hello World */
/* Hello
 * you
 */
/* a
 * really 
 * realy 
 * reajklfdj
 * rjklfd
 */

import (
    "fmt"
    )

var string_literal_bugfix []string = []string{"\\", "\"", "\"\"", "\"abc\"", "\"abc\"\\\"", "\"abc\"\\\n\"", `ab\n
cd`, "日本語", "\u65e5本\U00008a9e", "\xff\u00FF"}
func main(){
    var m int32 = 0X5
    m = 0x100
    m = 0124
    m = 12

    var n float64
    n = 0.
    n = 72.40
    n = 072.40  // == 72.40
    n = 2.71828
    n = 1.e+0
    n = 6.67428e-11
    n = 1E6
    n = .25
    n = .12345E+5

    var r rune
    r = 'a'
    r = 'ä'
    r = '本'
    r = '\t'
    r = '\000'
    r = '\007'
    r = '\377'
    r = '\x07'
    r = '\xff'
    r = '\u12e4'
    r = '\U00101234'

    fmt.Printf("this is just a test, %d %f %c\n", m, n, r)
    fmt.Println(string_literal_bugfix)
}
