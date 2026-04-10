spec main() {
    show("Enter a number (type 42):")
    firm x = ask { show("") }
    show("Type detected: " + kind(x))
    check (kind(x) == "Num") {
        show("✓ SUCCESS: Number detected correctly!")
        show("Value: " + convert x to Text)
    } altern {
        show("✗ FAILED: Expected Num but got " + kind(x))
    }
}

funcall main()
