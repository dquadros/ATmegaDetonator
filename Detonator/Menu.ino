/*
 * ATmegaDetonator - Seleção por Menu
 * 
 * (C) 2020, Daniel Quadros
 */

const int nlinMenu = 3;

/**
 * Rotina para apresentar o menu e ler uma opção
 */
int leMenu (MENU *menu) {
  int topo = 0;
  int old_topo = -1;
  int sel = 0;
  int n = menu->nItens;
  if (n > nlinMenu) {
    n = nlinMenu;
  }
  int ant = -1;
  Display_clear();
  Display_print (0, 0, menu->titulo, 0);
  while (true) {
      if (sel != ant) {
        // Rola se necessário
        if (sel < topo) {
          topo = sel;
        }
        if (sel > (topo+nlinMenu-1)) {
          topo = sel - nlinMenu +1;
        }
        // Redesenha o menu
        if (topo != old_topo) {
          for (int i = 0; i < nlinMenu; i++) {
            int opc = topo+i;
            Display_clearline (i+1);
            if (opc == sel) {
              Display_print (i+1, 0, menu->opcoes[opc], VID_REVERSO);
            } else {
              Display_print (i+1, 0, menu->opcoes[opc], VID_NORMAL);
            }
          }
          old_topo = topo;
        } else {
          Display_print (ant-topo+1, 0, menu->opcoes[ant], VID_NORMAL);
          Display_print (sel-topo+1, 0, menu->opcoes[sel], VID_REVERSO);
        }
        ant = sel;
      }
      switch (tiraFilaEnc()) {
        case ENTER:
          return sel;
        case UP:
          if (sel > 0) {
            sel--;
          }
          break;
       case DOWN:
          if (sel < (menu->nItens-1)) {
            sel++;
          }
      }
  }
}
