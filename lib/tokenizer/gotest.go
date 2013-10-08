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
    m = 0
    m = 0x100
    m = 0124
    m = -12
    m = 13

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

    var t complex64
    t = 0 + 0i
    t = 0 + 011i  // == 11i
    t = 0 + 0.i
    t = 0 + 2.71828i
    t = 0 + 1.e+0i
    t = 0 + 6.67428e-11i
    t = 0 + 1E6i
    t = 0 + .25i
    t = 0 + .12345E+5i

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

    fmt.Printf("this is just a test, %d %f %c %f\n", m, n, r, t)
    fmt.Println(string_literal_bugfix)
}
