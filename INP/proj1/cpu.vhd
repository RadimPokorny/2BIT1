-- cpu.vhd: Simple 8-bit CPU (BrainFuck interpreter)
-- Copyright (C) 2025 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Radim Pokorný <xpokorr00 AT stud.fit.vutbr.cz>
--
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
  port (
    CLK   : in std_logic; -- hodinovy signal
    RESET : in std_logic; -- asynchronni reset procesoru
    EN    : in std_logic; -- povoleni cinnosti procesoru

    -- synchronni pamet RAM
    DATA_ADDR  : out std_logic_vector(12 downto 0); -- adresa do pameti
    DATA_WDATA : out std_logic_vector(7 downto 0); -- mem[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
    DATA_RDATA : in std_logic_vector(7 downto 0); -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
    DATA_RDWR  : out std_logic; -- cteni (1) / zapis (0)
    DATA_EN    : out std_logic; -- povoleni cinnosti

    -- vstupni port
    IN_DATA : in std_logic_vector(7 downto 0); -- IN_DATA <- stav klavesnice pokud IN_VLD='1' a IN_REQ='1'
    IN_VLD  : in std_logic; -- data platna
    IN_REQ  : out std_logic; -- pozadavek na vstup data

    -- vystupni port
    OUT_DATA : out std_logic_vector(7 downto 0); -- zapisovana data
    OUT_BUSY : in std_logic; -- LCD je zaneprazdnen (1), nelze zapisovat
    OUT_INV  : out std_logic; -- pozadavek na aktivaci inverzniho zobrazeni (1)
    OUT_WE   : out std_logic; -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'

    -- stavove signaly
    READY : out std_logic; -- hodnota 1 znamena, ze byl procesor inicializovan a zacina vykonavat program
    DONE  : out std_logic -- hodnota 1 znamena, ze procesor ukoncil vykonavani programu (narazil na instrukci STOP)
  );
end cpu;

-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of cpu is
  -- pri tvorbe kodu reflektujte rady ze cviceni INP, zejmena mejte na pameti, ze 
  --   - nelze z vice procesu ovladat stejny signal,
  --   - je vhodne mit jeden proces pro popis jedne hardwarove komponenty, protoze pak
  --      - u synchronnich komponent obsahuje sensitivity list pouze CLK a RESET a 
  --      - u kombinacnich komponent obsahuje sensitivity list vsechny ctene signaly. 
  -- 1. Multiplexor
  signal MPX1 : std_logic;
  -- 2. Multiplexor
  signal MPX2 : std_logic_vector(1 downto 0);
  -- Signály pro řízení čítačů a registrů
  signal P_CNT_INC, P_CNT_DEC : std_logic;
  signal CNT_INC, CNT_DEC     : std_logic;
  signal PTR_INC, PTR_DEC     : std_logic;
  signal P_CNT, CNT, PTR      : std_logic_vector(12 downto 0) := (others => '0'); -- inicializace čítačů a ukazatele
  type states is (
    START, -- Inicializační stav
    IDLE, -- Čekání na START
    LOAD, -- Načtení instrukce
    DECODE, -- Dekódování instrukce
    MOV_R, -- >
    MOV_L, -- <
    INC, INC_W, -- Inkrementace
    DEC, DEC_W, -- Dekrementace
    PRINT, PRINT_CHAR, -- výpis znaků
    READ_IN, -- , čtení vstupu
    -- While smyčka stavy
    WHILE_START, WHILE_MEM_IS_ZERO, WHILE_CNT_NON_ZERO,
    WHILE_CNT_MOD, WHILE_IS_END_Q, WHILE_END, WHILE_MEM_IS_ZERO_E,
    WHILE_CNT_NON_ZERO_E, WHILE_CNT_MOD_E, WHILE_IS_END_Q_E,
    -- Do-while smyčka stavy
    LOOP_START, LOOP_MEM_IS_ZERO, LOOP_CNT_NON_ZERO, LOOP_CNT_MOD, LOOP_IS_END_Q, LOOP_END,
    LOOP_MEM_IS_ZERO_E, LOOP_CNT_NON_ZERO_E, LOOP_CNT_MOD_E, LOOP_IS_END_Q_E,
    STOP, -- ["@"] konec programu
    NO_OP -- neznámá instrukce (NOP)
  );
  signal STATE      : states := IDLE; -- aktivní stav
  signal NEXT_STATE : states; -- následující stav
  procedure set_read_defaults(
    signal s_DATA_EN   : out std_logic;
    signal s_DATA_RDWR : out std_logic;
    signal s_MPX1      : out std_logic
  ) is
  begin
    s_DATA_EN   <= '1'; -- povolení čtení
    s_DATA_RDWR <= '1'; -- čtecí režim
    s_MPX1      <= '1'; -- získat data
  end procedure;
  procedure set_read_mem_defaults(
    signal s_DATA_EN   : out std_logic;
    signal s_DATA_RDWR : out std_logic;
    signal s_MPX1      : out std_logic
  ) is
  begin
    s_DATA_EN   <= '1'; -- povolení čtení
    s_DATA_RDWR <= '1'; -- čtecí režim
    s_MPX1      <= '0'; -- čtení z P_CNT
  end procedure;
  procedure set_write_defaults(
    signal s_DATA_EN   : out std_logic;
    signal s_DATA_RDWR : out std_logic;
    signal s_MPX1      : out std_logic
  ) is
  begin
    s_DATA_EN   <= '1'; -- povolení čtení
    s_DATA_RDWR <= '0'; -- čtecí režim
    s_MPX1      <= '1'; -- čtení z P_CNT
  end procedure;
  procedure inc_and_load(
    signal s_P_CNT_INC  : out std_logic;
    signal s_NEXT_STATE : out states
  ) is
  begin
    s_P_CNT_INC  <= '1'; -- posun na další instrukci
    s_NEXT_STATE <= LOAD; -- přejít do LOAD stavu
  end procedure;
begin
  -- multiplexory
  MX1 : process (P_CNT, PTR, MPX1)
  begin
    if MPX1 = '0' then
      DATA_ADDR <= P_CNT;
    else
      DATA_ADDR <= PTR;
    end if;
  end process;
  MX2 : process (DATA_RDATA, IN_DATA, MPX2)
  begin
    case MPX2 is
      when "01" => DATA_WDATA <= DATA_RDATA - 1;
      when "10" => DATA_WDATA <= DATA_RDATA + 1;
      when "11" =>
        case DATA_RDATA is
          when x"30"  => DATA_WDATA  <= x"00"; -- '0' -> 0x00
          when x"31"  => DATA_WDATA  <= x"10"; -- '1' -> 0x10
          when x"32"  => DATA_WDATA  <= x"20"; -- '2' -> 0x20
          when x"33"  => DATA_WDATA  <= x"30"; -- '3' -> 0x30
          when x"34"  => DATA_WDATA  <= x"40"; -- '4' -> 0x40
          when x"35"  => DATA_WDATA  <= x"50"; -- '5' -> 0x50
          when x"36"  => DATA_WDATA  <= x"60"; -- '6' -> 0x60
          when x"37"  => DATA_WDATA  <= x"70"; -- '7' -> 0x70
          when x"38"  => DATA_WDATA  <= x"80"; -- '8' -> 0x80
          when x"39"  => DATA_WDATA  <= x"90"; -- '9' -> 0x90
          when x"41"  => DATA_WDATA  <= x"A0"; -- 'A' -> 0xA0
          when x"42"  => DATA_WDATA  <= x"B0"; -- 'B' -> 0xB0
          when x"43"  => DATA_WDATA  <= x"C0"; -- 'C' -> 0xC0
          when x"44"  => DATA_WDATA  <= x"D0"; -- 'D' -> 0xD0
          when x"45"  => DATA_WDATA  <= x"E0"; -- 'E' -> 0xE0
          when x"46"  => DATA_WDATA  <= x"F0"; -- 'F' -> 0xF0
          when others => DATA_WDATA <= IN_DATA; -- ostatní znaky
        end case;
      when others => DATA_WDATA <= DATA_RDATA; -- výchozí hodnota
    end case;
  end process;
  COUNTERS : process (CLK, RESET)
  begin
    if RESET = '1' then
      P_CNT <= (others => '0');
      CNT   <= (others => '0');
    elsif rising_edge(CLK) then
      -- Programový čítač
      if P_CNT_INC = '1' then
        P_CNT <= P_CNT + 1;
      elsif P_CNT_DEC = '1' then
        P_CNT <= P_CNT - 1;
      end if;
      -- Čítač pro cyklus
      if CNT_INC = '1' then
        CNT <= CNT + 1;
      elsif CNT_DEC = '1' then
        CNT <= CNT - 1;
      end if;
    end if;
  end process;
  MEM_PTR : process (CLK, RESET)
  begin
    if (RESET = '1') then
      PTR <= (others => '0');
    elsif rising_edge(CLK) then
      if (PTR_INC = '1') then
        PTR <= PTR + 1;
      elsif (PTR_DEC = '1') then
        PTR <= PTR - 1;
      end if;
    end if;
  end process;
  process (CLK, RESET)
  begin
    if RESET = '1' then
      STATE <= START;
    elsif rising_edge(CLK) then
      if EN = '1' then
        STATE <= NEXT_STATE;
      end if;
    end if;
  end process;
  NEXT_STATE_LOGIC : process (STATE, DATA_RDATA, OUT_BUSY, IN_VLD)
  begin
    if (RESET = '1') then
      OUT_DATA  <= (others => '0');
      DATA_EN   <= '0';
      DATA_RDWR <= '0';
      DONE      <= '0';
      READY     <= '0';
    end if;
    -- výchozí hodnoty signálů
    OUT_WE    <= '0';
    IN_REQ    <= '0';
    P_CNT_INC <= '0';
    P_CNT_DEC <= '0';
    CNT_INC   <= '0';
    CNT_DEC   <= '0';
    PTR_INC   <= '0';
    PTR_DEC   <= '0';
    case STATE is
      when START =>
        READY      <= '0';
        NEXT_STATE <= IDLE;
      when IDLE =>
        -- čeká se na "@" znak (0x40) pro START
        if DATA_RDATA = x"40" then
          READY      <= '1';
          NEXT_STATE <= LOAD;
        else
          PTR_INC <= '1';
          set_read_defaults(DATA_EN, DATA_RDWR, MPX1);
        end if;
      when LOAD =>
        set_read_mem_defaults(DATA_EN, DATA_RDWR, MPX1);
        NEXT_STATE <= DECODE;
      when DECODE =>
        case (DATA_RDATA) is
          when x"3E"         => NEXT_STATE         <= MOV_R; -- >
          when x"3C"         => NEXT_STATE         <= MOV_L; -- <
          when x"2B"         => NEXT_STATE         <= INC; -- +
          when x"2D"         => NEXT_STATE         <= DEC; -- -
          when x"5B"         => NEXT_STATE         <= WHILE_START; -- [
          when x"5D"         => NEXT_STATE         <= WHILE_END; -- ]
          when x"28"         => NEXT_STATE         <= LOOP_START; -- (
          when x"29"         => NEXT_STATE         <= LOOP_END; -- )
          when x"2E"         => NEXT_STATE         <= PRINT; -- .
          when x"2C"         => NEXT_STATE         <= READ_IN; -- ,
          when x"40" | x"00" => NEXT_STATE <= STOP; -- "@"
          when others        => NEXT_STATE        <= NO_OP; -- NOP pro neznámé instrukce
        end case;
      when MOV_R =>
        PTR_INC <= '1'; -- Inkrementace ukazatele
        inc_and_load(P_CNT_INC, NEXT_STATE);
      when MOV_L =>
        PTR_DEC <= '1'; -- Dekrementace ukazatele
        inc_and_load(P_CNT_INC, NEXT_STATE);
      when INC =>
        set_read_defaults(DATA_EN, DATA_RDWR, MPX1);
        NEXT_STATE <= INC_W; -- přejít na zápis
      when INC_W =>
        set_write_defaults(DATA_EN, DATA_RDWR, MPX1);
        MPX2 <= "10"; -- inkrementace
        inc_and_load(P_CNT_INC, NEXT_STATE);
      when DEC =>
        set_read_defaults(DATA_EN, DATA_RDWR, MPX1);
        NEXT_STATE <= DEC_W; -- přejít na zápis
      when DEC_W =>
        set_write_defaults(DATA_EN, DATA_RDWR, MPX1);
        MPX2 <= "01"; -- dekrementace
        inc_and_load(P_CNT_INC, NEXT_STATE);
      when WHILE_START =>
        P_CNT_INC <= '1'; -- posun na další instrukci
        set_read_defaults(DATA_EN, DATA_RDWR, MPX1);
        NEXT_STATE <= WHILE_MEM_IS_ZERO; -- pokračovat na další instrukci
      when WHILE_MEM_IS_ZERO =>
        if (DATA_RDATA = "00000000") then
          CNT_INC <= '1'; -- Inkrementace čítače
          set_read_mem_defaults(DATA_EN, DATA_RDWR, MPX1);
          NEXT_STATE <= WHILE_CNT_NON_ZERO; -- Začít smyčku
        else
          NEXT_STATE <= LOAD; -- Skončit smyčku
        end if;
      when WHILE_CNT_NON_ZERO =>
        set_read_mem_defaults(DATA_EN, DATA_RDWR, MPX1);
        NEXT_STATE <= WHILE_CNT_MOD; -- Pokračovat ve smyčce
      when WHILE_CNT_MOD =>
        case (DATA_RDATA) is -- Inkrementace/dekrementace čítače podle instrukce
          when x"5B"  => CNT_INC <= '1'; -- [ 
          when x"5D"  => CNT_DEC <= '1'; -- ]
          when others => null;
        end case;
        NEXT_STATE <= WHILE_IS_END_Q; -- Kontrola konce smyčky
      when WHILE_IS_END_Q =>
        P_CNT_INC <= '1'; -- posun na další instrukci
        if (CNT = "00000000") then
          NEXT_STATE <= LOAD; -- Konec, jestliže je čítač nulový
        else
          NEXT_STATE <= WHILE_CNT_NON_ZERO; -- Pokračovat ve smyčce
        end if;
      when WHILE_END =>
        set_read_defaults(DATA_EN, DATA_RDWR, MPX1);
        NEXT_STATE <= WHILE_MEM_IS_ZERO_E; -- pokračovat na další instrukci
      when WHILE_MEM_IS_ZERO_E =>
        if (DATA_RDATA = "00000000") then
          inc_and_load(P_CNT_INC, NEXT_STATE);
        else
          CNT_INC   <= '1'; -- Inkrementace čítače
          P_CNT_DEC <= '1'; -- Decrementace P_CNT pro návrat na začátek smyčky
          set_read_mem_defaults(DATA_EN, DATA_RDWR, MPX1);
          NEXT_STATE <= WHILE_CNT_NON_ZERO_E; -- Začít smyčku
        end if;
      when WHILE_CNT_NON_ZERO_E =>
        set_read_mem_defaults(DATA_EN, DATA_RDWR, MPX1);
        NEXT_STATE <= WHILE_CNT_MOD_E; -- Pokračovat ve smyčce
      when WHILE_CNT_MOD_E =>
        case (DATA_RDATA) is -- Inkrementace/dekrementace čítače podle instrukce
          when x"5B"  => CNT_DEC <= '1'; -- [
          when x"5D"  => CNT_INC <= '1'; -- ] 
          when others => null;
        end case;
        NEXT_STATE <= WHILE_IS_END_Q_E; -- Kontrola konce smyčky
      when WHILE_IS_END_Q_E =>
        if (CNT = "00000000") then
          inc_and_load(P_CNT_INC, NEXT_STATE);
        else
          P_CNT_DEC  <= '1'; -- posun zpět (dekrementace)
          NEXT_STATE <= WHILE_CNT_NON_ZERO_E; -- Pokračovat ve smyčce
        end if;
      when LOOP_START =>
        inc_and_load(P_CNT_INC, NEXT_STATE);
      when LOOP_END => -- Teprve teď podmínku vyhodnocujeme
        set_read_defaults(DATA_EN, DATA_RDWR, MPX1);
        NEXT_STATE <= LOOP_MEM_IS_ZERO_E; -- pokračovat na další instrukci
      when LOOP_MEM_IS_ZERO_E =>
        if (DATA_RDATA = "00000000") then -- Jestliže je hodnota 0, pokračujeme dál
          inc_and_load(P_CNT_INC, NEXT_STATE);
        else
          CNT_INC   <= '1'; -- Inkrementace čítače
          P_CNT_DEC <= '1'; -- Decrementace P_CNT pro návrat na konec do-while
          set_read_mem_defaults(DATA_EN, DATA_RDWR, MPX1);
          NEXT_STATE <= LOOP_CNT_NON_ZERO_E; -- Začít smyčku
        end if;
      when LOOP_CNT_NON_ZERO_E =>
        set_read_mem_defaults(DATA_EN, DATA_RDWR, MPX1);
        NEXT_STATE <= LOOP_CNT_MOD_E; -- Pokračovat ve smyčce
      when LOOP_CNT_MOD_E =>
        case (DATA_RDATA) is -- Inkrementace/dekrementace čítače podle instrukce
          when x"28"  => CNT_DEC <= '1'; -- (
          when x"29"  => CNT_INC <= '1'; -- )
          when others => null;
        end case;
        NEXT_STATE <= LOOP_IS_END_Q_E; -- Kontrola konce smyčky
      when LOOP_IS_END_Q_E =>
        if (CNT = "00000000") then -- našel se konec smyčky
          inc_and_load(P_CNT_INC, NEXT_STATE);
        else
          P_CNT_DEC  <= '1'; -- posun zpět (dekrementace. Hledání pokračuje)
          NEXT_STATE <= LOOP_CNT_NON_ZERO_E; -- Pokračovat ve smyčce
        end if;
      when PRINT =>
        DATA_EN    <= '1'; -- Povolení čtení
        MPX1       <= '1'; -- Čtení z paměti
        NEXT_STATE <= PRINT_CHAR; -- pokračovat na další instrukci
      when PRINT_CHAR => -- odeslání dat na výstupní port
        if (OUT_BUSY = '1') then -- LCD je zaneprázdněn, čekat
          DATA_EN    <= '1'; -- Povolení čtení
          MPX1       <= '1'; -- Čtení z paměti
          NEXT_STATE <= PRINT_CHAR; -- čekat, dokud nebude LCD volné
        else
          OUT_DATA <= DATA_RDATA; -- připravit data na výstup
          OUT_WE   <= '1'; -- zapsat na LCD
          inc_and_load(P_CNT_INC, NEXT_STATE);
        end if;
      when READ_IN =>
        IN_REQ <= '1'; -- požadavek na vstup dat
        if (IN_VLD = '1') then -- data jsou platná, můžeme je číst
          NEXT_STATE <= NO_OP; -- přejít na zápis do paměti
        end if;
      when NO_OP =>
        set_write_defaults(DATA_EN, DATA_RDWR, MPX1);
        MPX2 <= "11"; -- Kód pro zápis hexadecimálních znaků
        inc_and_load(P_CNT_INC, NEXT_STATE);
      when STOP =>
        DONE <= '1'; -- Konec programu
      when others =>
        NEXT_STATE <= IDLE; -- výchozí návrat do IDLE (nejspíš nenastane)
    end case;
  end process;
end behavioral;
