#ifndef BSP_LED_H
#define BSP_LED_H
/* Demo GPIOs PF9/PF10 follow the existing board configuration.
 * LED wiring/polarity must be checked against the actual board schematic.
 */
void BSP_LED_Init(void);
void BSP_LED_ToggleAll(void);
#endif
