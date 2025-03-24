import { render, screen } from '@testing-library/react';
import App from './App';

test('renderiza a tela de carregamento', () => {
  render(<App />);
  const loadingText = screen.getByText('Carregando Mega Emu...');
  expect(loadingText).toBeInTheDocument();
});
