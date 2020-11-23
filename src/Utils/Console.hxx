/*--------------------------------------------------------------------------*\
 |                                                                          |
 |  Copyright (C) 2020                                                      |
 |                                                                          |
 |         , __                 , __                                        |
 |        /|/  \               /|/  \                                       |
 |         | __/ _   ,_         | __/ _   ,_                                |
 |         |   \|/  /  |  |   | |   \|/  /  |  |   |                        |
 |         |(__/|__/   |_/ \_/|/|(__/|__/   |_/ \_/|/                       |
 |                           /|                   /|                        |
 |                           \|                   \|                        |
 |                                                                          |
 |      Enrico Bertolazzi                                                   |
 |      Dipartimento di Ingegneria Industriale                              |
 |      Universita` degli Studi di Trento                                   |
 |      email: enrico.bertolazzi@unitn.it                                   |
 |                                                                          |
\*--------------------------------------------------------------------------*/

///
/// file: Console.hxx
///

#pragma once

#ifndef CONSOLE_dot_HXX
#define CONSOLE_dot_HXX

namespace Utils {

  typedef std::basic_ostream<char> ostream_type;

  std::string basename( char const filename[] );

  class Console {

    mutable std::mutex message_mutex; // mutex for critical section

  public:
    class Console_style {
    public:
      rang::style s;
      rang::fg    f;
      rang::bg    b;
    };

  private:

    ostream_type * p_stream;

    // 0 only fatal, error
    // 1 + warning
    // 2
    int level;

    Console_style message_style;
    Console_style warning_style;
    Console_style error_style;
    Console_style fatal_style;

    Console() = delete;
    Console( Console const & ) = delete;

  public:

    Console( ostream_type * p_stream = &std::cout, int level = 4 );

    void changeLevel( int new_level );
    void changeStream( ostream_type * new_p_stream );
    int  getLevel() const { return level; }

    ostream_type * getStream() const { return p_stream; }

    Console const & flush() const { p_stream->flush(); return *this; }

    Console const &
    message( std::string const & msg, int msg_level = 4 ) const;

    Console const &
    semaphore( unsigned ryg, std::string const & msg, int msg_level = 0 ) const;

    Console const &
    warning( std::string const & msg ) const; // level >= 2

    Console const &
    error( std::string const & msg ) const; // level >= 1

    Console const &
    fatal( std::string const & msg ) const; // level >= 0

    Console const & black   ( std::string const & msg, int msg_level = 0 ) const;
    Console const & red     ( std::string const & msg, int msg_level = 0 ) const;
    Console const & green   ( std::string const & msg, int msg_level = 0 ) const;
    Console const & yellow  ( std::string const & msg, int msg_level = 0 ) const;
    Console const & blue    ( std::string const & msg, int msg_level = 0 ) const;
    Console const & magenta ( std::string const & msg, int msg_level = 0 ) const;
    Console const & cyan    ( std::string const & msg, int msg_level = 0 ) const;
    Console const & gray    ( std::string const & msg, int msg_level = 0 ) const;

    Console const & black_reversed   ( std::string const & msg, int msg_level = 0 ) const;
    Console const & red_reversed     ( std::string const & msg, int msg_level = 0 ) const;
    Console const & green_reversed   ( std::string const & msg, int msg_level = 0 ) const;
    Console const & yellow_reversed  ( std::string const & msg, int msg_level = 0 ) const;
    Console const & blue_reversed    ( std::string const & msg, int msg_level = 0 ) const;
    Console const & magenta_reversed ( std::string const & msg, int msg_level = 0 ) const;
    Console const & cyan_reversed    ( std::string const & msg, int msg_level = 0 ) const;
    Console const & gray_reversed    ( std::string const & msg, int msg_level = 0 ) const;

    void
    setMessageStyle(
      rang::style const & s,
      rang::fg    const & f,
      rang::bg    const & b
    ) {
      message_style.s = s;
      message_style.f = f;
      message_style.b = b;
    }

    void
    setWarningStyle(
      rang::style const & s,
      rang::fg    const & f,
      rang::bg    const & b
    ) {
      warning_style.s = s;
      warning_style.f = f;
      warning_style.b = b;
    }

    void
    setErrorStyle(
      rang::style const & s,
      rang::fg    const & f,
      rang::bg    const & b
    ) {
      error_style.s = s;
      error_style.f = f;
      error_style.b = b;
    }

    void
    setFatalStyle(
      rang::style const & s,
      rang::fg    const & f,
      rang::bg    const & b
    ) {
      fatal_style.s = s;
      fatal_style.f = f;
      fatal_style.b = b;
    }

    //! set off coloring
    void
    setOff() const {
      #ifndef UTILS_OS_WINDOWS
      rang::setControlMode( rang::control::Off );
      #endif
    }

    //! set coloring automatic
    void
    setAuto() const {
      #ifdef UTILS_OS_WINDOWS
      rang::setWinTermMode( rang::winTerm::Auto );
      #else
      rang::setControlMode( rang::control::Auto );
      #endif
    }

  };
}

#endif

///
/// eof: Console.hxx
///