spec main:
    show "Testing Signals..."
    
    listen "UserLogin" {
        show "Caught event: UserLogin"
    }
    
    show "Emitting event..."
    signal "UserLogin" {
        < Optional payload/body? >
    }
    
    show "Done."
done

main()
